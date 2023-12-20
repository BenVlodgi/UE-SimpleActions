// Copyright

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SimpleAction.h"


#include "SimpleActionList.generated.h"

/**
 *
 */

USTRUCT(BlueprintType)
struct SIMPLEACTIONS_API FSimpleActionList
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, meta = (ShowOnlyInnerProperties))
	TArray<TObjectPtr<USimpleAction>> Actions;

};

