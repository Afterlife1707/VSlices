#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Assassinatable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UAssassinatable : public UInterface
{
	GENERATED_BODY()
};

class VSLICES_API IAssassinatable
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Assassination")
	void OnAssassinated();
};