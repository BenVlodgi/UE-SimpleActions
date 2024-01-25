// Copyright


#include "SimpleAction.h"

#include "Engine/World.h"


DEFINE_LOG_CATEGORY(LogSimpleAction);


void USimpleAction::Tick(float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(USimpleAction::Tick);
    ActionTick(DeltaTime);
};

UWorld* USimpleAction::GetWorld() const
{
    //Return null if the called from the CDO, or if the outer is being destroyed
    if (HasAnyFlags(RF_ClassDefaultObject) || GetOuter()->HasAnyFlags(RF_BeginDestroyed) || GetOuter()->IsUnreachable())
    {
        return nullptr;
    }

    //Try to get the world from the owning actor if we have one
    if (const AActor* Outer = GetTypedOuter<AActor>())
    {
        return Outer->GetWorld();
    }
    
    if(ActingActor.IsValid())
    {
		return ActingActor->GetWorld();
	}

    if (ActionInstigator.IsValid())
    {
        return ActionInstigator->GetWorld();
    }

    //Else return null - the latent action will fail to initialize
    return nullptr;
}

FString USimpleAction::GetActionUID()
{

    if(ActionUID.IsEmpty())
    {
	    ActionUID = TEXT("" + FGuid::NewGuid().ToString());
    }
	return ActionUID;

}

#if WITH_EDITOR
void USimpleAction::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
        TRACE_CPUPROFILER_EVENT_SCOPE(USimpleAction::ActionEditorPropertyChange);
        ActionEditorPropertyChange();
	}
}
#endif //WITH_EDITOR

void USimpleAction::PostInitProperties()
{
#if WITH_EDITOR && WITH_EDITORONLY_DATA

	// We do not run the default initialization for objects that are CDOs.
    EObjectFlags ExcludeFlags = RF_ClassDefaultObject;

	if (!bHasBeenInitialized && !HasAnyFlags(ExcludeFlags))
	{
        TRACE_CPUPROFILER_EVENT_SCOPE(USimpleAction::ActionEditorInitialize);
        ActionEditorInitialize();
		bHasBeenInitialized = true;
	}
#endif

	Super::PostInitProperties();
}

void USimpleAction::ActionEditorInitialize_Implementation()
{
}

void USimpleAction::ActionEditorPropertyChange_Implementation()
{
}

void USimpleAction::StartAction(UPARAM(DisplayName = "ActionActor") AActor* NewActionActor, UPARAM(DisplayName = "Instigator") AActor* NewInstigator)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(USimpleAction::StartAction);

    bool bFailed = false;
    UWorld* World = GetWorld();
    if (!World)
    {
        // Only run in valid worlds.
        UE_LOG(LogSimpleAction, Warning, TEXT("StartAction: Failed. Action '%s' world not valid."), *GetName());
		bFailed = true;
    }

    if (!bFailed && !bAllowInEditorPreview && World->IsPreviewWorld())
    {
		// This Action isn't allowed to run in editor worlds.
		UE_LOG(LogSimpleAction, Warning, TEXT("StartAction: Failed. Action '%s' not allowed in Editor Preview World: %s."), *GetName(), LexToString(GetWorld()->WorldType));
        bFailed = true;
    }
    if (!bFailed && !bAllowInLevelEditor && World->WorldType == EWorldType::Type::Editor)
    {
        // This Action isn't allowed to run in editor worlds.
        UE_LOG(LogSimpleAction, Warning, TEXT("StartAction: Failed. Action '%s' not allowed in Editor World: %s."), *GetName(), LexToString(GetWorld()->WorldType));
        bFailed = true;
    }

    if(bFailed) {
	    OnActionFailed.Broadcast(this);
    }

    if (bActive)
    {
        if (bAllowDoubleStart)
        {
            // Cancel the running action before starting a new one.
            CancelAction();
        }
        else
        {
            // Already active, don't restart.
            UE_LOG(LogSimpleAction, Verbose, TEXT("StartAction: Failed. Action '%s' already active."), *GetName());
            return;
        }
    }

    bActive = true;

    ActingActor = NewActionActor;
    ActionInstigator = NewInstigator;

    if (bAutoEnableTickWhileActive)
    {
        SetTickEnabled(true);
    }

    UE_LOG(LogSimpleAction, Verbose, TEXT("StartAction: Starting. Action '%s'."), *GetName());

    OnActionStarted.Broadcast(this);

    if(DurationOverride > 0.f)
    {
	    const FTimerDelegate DurationOverrideDelegate = FTimerDelegate::CreateUObject(this, &USimpleAction::FinishAction, true);
        GetWorld()->GetTimerManager().SetTimer(Timer_DurationOverride, DurationOverrideDelegate, DurationOverride + StartDelay, false);
    }

    if(StartDelay <= 0)
    {
		ActionStart(); 
    } else {
        const FTimerDelegate StartDelayDelegate = FTimerDelegate::CreateUObject(this, &USimpleAction::ActionStart);
        GetWorld()->GetTimerManager().SetTimer(Timer_StartDelay, StartDelayDelegate, StartDelay, false);
    }
}

void USimpleAction::StartAction_Bound(AActor* NewActionActor, AActor* NewInstigator, FOnActionStartedDelegate Started, FOnActionCancelDelegate Interrupted, FOnActionEndedDelegate Ended, FOnActionCompleteDelegate Complete, FOnActionFailedDelegate Failed) {
    
    OnActionStarted.AddUnique(Started);
    OnActionCancel.AddUnique(Interrupted);
    OnActionEnded.AddUnique(Ended);
    OnActionComplete.AddUnique(Complete);
    OnActionFailed.AddUnique(Failed);

    StartAction(NewActionActor, NewInstigator);
}

void USimpleAction::FinishAction(bool bSuccess)
{

    // Only prevents the EndAction if there exists a DurationOverride timer active.
    if(DurationOverride > 0.f) {
        const FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	    if (TimerManager.GetTimerRemaining(Timer_DurationOverride) > 0.f) {
		    return;
	    }
    }

    EndAction(bSuccess);
}

void USimpleAction::CancelAction()
{

    OnActionCancel.Broadcast(this);
    EndAction(false);
}

void USimpleAction::EndAction(bool bSuccess)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(USimpleAction::EndAction);

    if (!bActive)
    {
        // Not active, can't end.
        UE_LOG(LogSimpleAction, Verbose, TEXT("EndAction: Failed. Action '%s' isn't active."), *GetName());
        return;
    }

    // Kill all timers if the action is ending
    if (StartDelay > 0.f) {
        GetWorld()->GetTimerManager().ClearTimer(Timer_StartDelay);
    }

    if (DurationOverride > 0.f) {
        GetWorld()->GetTimerManager().ClearTimer(Timer_DurationOverride);
    }

    bActive = false;

    SetTickEnabled(false);
    UE_LOG(LogSimpleAction, Verbose, TEXT("EndAction: Stopping. Action '%s'. Success: %s."), *GetName(), bSuccess ? TEXT("True") : TEXT("False"));

    if(bStopWhenActiveStarted)
    {
		ActionStop(bSuccess);
    }

    if(bSuccess)
    {
	    OnActionComplete.Broadcast(this);
    }
	else
    {
		OnActionFailed.Broadcast(this);
	}

    OnActionEnded.Broadcast(this, bSuccess);

    // Final cleanup
    ActionReset();

    // These are left set until now in case the OnActionEnded Broadcast or ActionReset wanted to have access to them.
    ActingActor = nullptr;
    ActionInstigator = nullptr;

}

void USimpleAction::SetTickEnabled(bool bTickEnabled)
{
    if (!bCanTick)
    {
		return;
	}

    // If we are not active, we can't tick.
    bTickManuallyEnabled = bTickEnabled && bActive;
}

UPARAM(DisplayName = "TickEnabled") bool USimpleAction::GetTickEnabled()
{
    return bTickManuallyEnabled;
}

void USimpleAction::ActionStart_Implementation()
{
}

void USimpleAction::ActionStop_Implementation(bool bSuccess)
{
}

void USimpleAction::ActionReset_Implementation()
{
}

void USimpleAction::ActionTick_Implementation(float DeltaTime)
{
}
