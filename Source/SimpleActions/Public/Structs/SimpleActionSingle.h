// Copyright

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SimpleAction.h"


#include "SimpleActionSingle.generated.h"

/**
 *
 */

USTRUCT(BlueprintType)
struct SIMPLEACTIONS_API FSimpleActionSingle
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TObjectPtr<USimpleAction> Action;

	FSimpleActionSingle() : FSimpleActionSingle(nullptr){}
	FSimpleActionSingle(USimpleAction* Action) {
		this->Action = Action;
	}

};

