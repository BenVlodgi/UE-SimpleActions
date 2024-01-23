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

    UWorld* World = GetWorld();
    if (!World)
    {
        // Only run in valid worlds.
        UE_LOG(LogSimpleAction, Warning, TEXT("StartAction: Failed. Action '%s' world not valid."), *GetName());
		return;
    }

    if (!bAllowInEditorPreview && World->IsPreviewWorld())
    {
		// This Action isn't allowed to run in editor worlds.
		UE_LOG(LogSimpleAction, Warning, TEXT("StartAction: Failed. Action '%s' not allowed in Editor Preview World: %s."), *GetName(), LexToString(GetWorld()->WorldType));
		return;
    }
    if (!bAllowInLevelEditor && World->WorldType == EWorldType::Type::Editor)
    {
        // This Action isn't allowed to run in editor worlds.
        UE_LOG(LogSimpleAction, Warning, TEXT("StartAction: Failed. Action '%s' not allowed in Editor World: %s."), *GetName(), LexToString(GetWorld()->WorldType));
        return;
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
    ActionStart();
    OnActionStarted.Broadcast(this);
}

void USimpleAction::FinishAction(bool bSuccess)
{
    EndAction(bSuccess);
}

void USimpleAction::CancelAction()
{
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

    bActive = false;

    SetTickEnabled(false);
    UE_LOG(LogSimpleAction, Verbose, TEXT("EndAction: Stopping. Action '%s'. Success: %s."), *GetName(), bSuccess ? TEXT("True") : TEXT("False"));

    ActionStop(bSuccess);
    OnActionEnded.Broadcast(this, bSuccess);

    // These are left set until now incase the OnActionEnded broadcast wanted to have access to them.
    ActingActor = nullptr;
    ActionInstigator = nullptr;

    // Final cleanup
    ActionReset();
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
