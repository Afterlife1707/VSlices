#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LoggingMacros.h" //for child classes
#include "GameFramework/CharacterMovementComponent.h"
#include "ParkourComponentBase.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VSLICES_API UParkourComponentBase : public UActorComponent
{
public:
	GENERATED_BODY()
	
	UParkourComponentBase();
	
protected:
	virtual void BeginPlay() override;
	UPROPERTY()
	class AVSlicesCharacter* OwnerCharacter;
	UPROPERTY()
	UCharacterMovementComponent* MovementComponent;
};
