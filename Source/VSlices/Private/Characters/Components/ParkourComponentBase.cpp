#include "Characters/Components/ParkourComponentBase.h"
#include "Characters/VSlicesCharacter.h"

UParkourComponentBase::UParkourComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
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
			LOG_WARNING("Duplicate %s detected - only one is allowed. Remove it from the Components panel.", *GetClass()->GetName());
			DestroyComponent();
		}
	}
}
#endif

void UParkourComponentBase::BeginPlay()
{
	if (!IsActive()) return;
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
