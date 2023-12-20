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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, meta = (ShowOnlyInnerProperties))
	TObjectPtr<USimpleAction> Action;

};

