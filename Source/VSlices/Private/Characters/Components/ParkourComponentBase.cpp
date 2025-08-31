#include "Characters/Components/ParkourComponentBase.h"
#include "Characters/VSlicesCharacter.h"

UParkourComponentBase::UParkourComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UParkourComponentBase::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AVSlicesCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		LOG_ERROR("Invalid Character!");
		return;
	}
	MovementComponent =  MovementComponent = OwnerCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		LOG_ERROR("Invalid MovementComponent");
		return;
	}
}
