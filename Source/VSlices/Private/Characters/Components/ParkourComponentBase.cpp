#include "Characters/Components/ParkourComponentBase.h"
#include "Characters/VSlicesCharacter.h"

UParkourComponentBase::UParkourComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

#if WITH_EDITOR
void UParkourComponentBase::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (const AActor* Owner = GetOwner())
	{
		TArray<UActorComponent*> Components;
		Owner->GetComponents(GetClass(), Components);

		if (Components.Num() > 1)
		{
			LOG_WARNING("Duplicate %s detected on %s - only one is allowed. Removing duplicate.", *GetClass()->GetName(), *Owner->GetName());
			SetFlags(RF_Transient);
			UnregisterComponent();
			DestroyComponent();
		}
	}
}
#endif

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

void UParkourComponentBase::WarnMissingAsset(const FString& AssetName) const
{
	if (!AlreadyWarned.Contains(AssetName))
	{
		AlreadyWarned.Add(AssetName);
		LOG_WARNING("%s on %s: Missing asset - %s. Assign it in the Blueprint Details panel.", *GetClass()->GetName(), *GetOwner()->GetName(), *AssetName);
	}
}
