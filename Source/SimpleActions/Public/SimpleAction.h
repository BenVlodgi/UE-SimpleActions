// Copyright

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"


#include "SimpleAction.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionStartedSignature, USimpleAction*, SimpleAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionCancelSignature, USimpleAction*, SimpleAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionEndedSignature, USimpleAction*, SimpleAction, bool, Success);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionCompleteSignature, USimpleAction*, SimpleAction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionFailedSignature, USimpleAction*, SimpleAction);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnActionStartedDelegate, USimpleAction*, SimpleAction);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnActionCancelDelegate, USimpleAction*, SimpleAction);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnActionEndedDelegate, USimpleAction*, SimpleAction, bool, Success);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnActionCompleteDelegate, USimpleAction*, SimpleAction);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnActionFailedDelegate, USimpleAction*, SimpleAction);

DECLARE_LOG_CATEGORY_EXTERN(LogSimpleAction, Log, All);

/**
 * An action which can be performed by an actor.
 * CollapseCategories,
 */
UCLASS(Blueprintable, DefaultToInstanced, EditInlineNew, Abstract, CollapseCategories, AutoExpandCategories = "Default,Simple Action", meta = (DisplayName = "Simple Action", PrioritizeCategories = "Simple Action"))
class SIMPLEACTIONS_API USimpleAction : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FString ActionUID;

public:
	// When true: If this action is called to start while it is already active, it will start again.
	// When false: Additional calls to start while the action is active will be ignored.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Simple Action")
	bool bAllowDoubleStart = true;

	// When true: If the actions is called to start while already running it will call ActionStop.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Simple Action")
	bool bStopWhenActiveStarted = true;

	// True if this actor ticks while active.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Simple Action")
	bool bCanTick = false;

	// Tick will be enabled when the action starts.
	// Tick is always disabled when the action ends.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Simple Action", meta = (EditCondition = "bCanTick"))
	bool bAutoEnableTickWhileActive = false;

	// When true: This action can run in editor while the game isn't running.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Simple Action")
	bool bAllowInLevelEditor = false;

	// When true: This action can run in editor preview windows.
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Simple Action")
	bool bAllowInEditorPreview = false;

protected:
	// True if this actor ticks while active.
	UPROPERTY()
	bool bTickManuallyEnabled = false;

	// Timer Handle for Action Start Delay
	FTimerHandle Timer_StartDelay;

	// Timer Handle for Action Duration override
	FTimerHandle Timer_DurationOverride;

public:
	// True once the action starts, and false once it ends.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Simple Action")
	bool bActive = false;

	// The actor that which is performing this action.
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Simple Action")
	TWeakObjectPtr<AActor> ActingActor = nullptr;
	
	// The object which triggered this action.
	UPROPERTY(BlueprintReadOnly, Transient,Category = "Simple Action")
	TWeakObjectPtr<UObject> ActionInstigator = nullptr;

	// The amount of time before the Action Starts.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Action")
	float StartDelay;

	// The amount of time the action will be forced to last.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Simple Action")
	float DurationOverride;

	// Called when the action is started.
	UPROPERTY(BlueprintAssignable, Category = "Simple Action")
	FOnActionStartedSignature OnActionStarted;

	// Called when the action in Interrupted.
	UPROPERTY(BlueprintAssignable, Category = "Simple Action")
	FOnActionCancelSignature OnActionCancel;
	
	// Called when the action is Ended.
	UPROPERTY(BlueprintAssignable, Category = "Simple Action")
	FOnActionEndedSignature OnActionEnded;

	// Called when the action is Ended as successful.
	UPROPERTY(BlueprintAssignable, Category = "Simple Action")
	FOnActionCompleteSignature OnActionComplete;

	// Called when the action has Failed to start.
	UPROPERTY(BlueprintAssignable, Category = "Simple Action")
	FOnActionFailedSignature OnActionFailed;

	// Wrapper delegates used for binding to Multicast Delegates via blueprint exposed parameters
	// Called when the action is Started
	FOnActionStartedDelegate OnActionStartedDelegate;

	// Called when the action is Interrupted.
	FOnActionCancelDelegate OnActionCancelDelegate;

	// Called when the action is Ended.
	FOnActionEndedDelegate OnActionEndedDelegate;

	// Called when the action is Ended as Successful.
	FOnActionCompleteDelegate OnActionCompleteDelegate;

	// Called when the action has failed to start.
	FOnActionFailedDelegate OnActionFailedDelegate;

#if WITH_EDITORONLY_DATA
protected:
	// True once the action has been initialized.
	UPROPERTY()
	bool bHasBeenInitialized = false;
#endif

public:
	// FTickableGameObject Interface
	virtual void Tick(float DeltaTime);
	virtual bool IsTickableInEditor() const { return true; }
	virtual ETickableTickType GetTickableTickType() const override { return (HasAnyFlags(RF_ClassDefaultObject)) || !bCanTick ? ETickableTickType::Never : ETickableTickType::Conditional; };
	virtual TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(USimpleAction, STATGROUP_Tickables); }
	virtual bool IsTickable() const { return bActive && bTickManuallyEnabled && bCanTick; }
	// FTickableGameObject Interface

	// This enables the object to use Latent Events.
	virtual UWorld* GetWorld() const override;

	// Gets the unique generated ID for this Simple Action
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simple Action", meta = (ReturnDisplayName = "UID"))
	virtual FString GetActionUID();
	
	// UObject interface
	/**
	 * Called after the C++ constructor and after the properties have been initialized, including those loaded from config.
	 * This is called before any serialization or other setup has happened.
	 */
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR
	// UObject interface

public:

	// Called when the action is initialized in the editor.
	UFUNCTION(BlueprintNativeEvent, Category = "Simple Action", meta = (DisplayName = "Editor: Initialize", DevelopmentOnly))
	void ActionEditorInitialize();
	virtual void ActionEditorInitialize_Implementation();

	// Called when a property is changed in the editor.
	UFUNCTION(BlueprintNativeEvent, Category = "Simple Action", meta = (DisplayName = "Editor: Property Changed", DevelopmentOnly))
	void ActionEditorPropertyChange();
	virtual void ActionEditorPropertyChange_Implementation();

	// Call to start the action.
	UFUNCTION(BlueprintCallable, Category = "Simple Action", meta = (DefaultToSelf = "NewInstigator"))
	void StartAction(UPARAM(DisplayName = "ActionActor") AActor* NewActionActor, UPARAM(DisplayName = "Instigator") AActor* NewInstigator);

	// Call to start the action with given functions to bind to related delegate broadcasts
	UFUNCTION(BlueprintCallable, Category = "Simple Action", meta = (DefaultToSelf = "NewInstigator"))
	void StartAction_Bound(UPARAM(DisplayName = "ActionActor") AActor* NewActionActor, UPARAM(DisplayName = "Instigator") AActor* NewInstigator, FOnActionStartedDelegate Started, FOnActionCancelDelegate Interrupted, FOnActionEndedDelegate Ended, FOnActionCompleteDelegate Complete, FOnActionFailedDelegate Failed);

	// Call to end the action with a status of successful.
	UFUNCTION(BlueprintCallable, Category = "Simple Action", meta = (keywords = "complete, end, stop"))
	void FinishAction(bool bSuccess = true);

	// Call to end the action with bSuccess = false.
	UFUNCTION(BlueprintCallable, Category = "Simple Action", meta = (keywords = "interrupt, end"))
	void CancelAction();

	UFUNCTION()
	void EndAction(bool bSuccess = true);


	// Set tick enabled state. Only works if bCanTick is true.
	// This will be disabled when the action ends.
	// No ticking happens when an action is not active.
	UFUNCTION(BlueprintCallable, Category = "Simple Action")
	void SetTickEnabled(bool bTickEnabled);

	// Get tick enabled state.
	// Even if this is true, that does not mean the action is ticking.
	UFUNCTION(BlueprintCallable, Category = "Simple Action")
	UPARAM(DisplayName = "TickEnabled") bool GetTickEnabled();

	// Called when the action is starting.
	UFUNCTION(BlueprintNativeEvent, Category = "Simple Action", meta = (DisplayName = "Action: Start"))
	void ActionStart();
	virtual void ActionStart_Implementation();

	// Called when the action is ending.
	// 
	// @param bSuccess false when the action was canceled.
	UFUNCTION(BlueprintNativeEvent, Category = "Simple Action", meta = (DisplayName = "Action: Stop"))
	void ActionStop(bool bSuccess);
	virtual void ActionStop_Implementation(bool bSuccess);

	// Called after the action has completely ended. Use for any final cleanup.
	UFUNCTION(BlueprintNativeEvent, Category = "Simple Action", meta = (DisplayName = "Action: Reset"))
	void ActionReset();
	virtual void ActionReset_Implementation();

	// Called when the action is starting.
	UFUNCTION(BlueprintNativeEvent, Category = "Simple Action", meta = (DisplayName = "Action: Tick"))
	void ActionTick(float DeltaTime);
	virtual void ActionTick_Implementation(float DeltaTime);

};
