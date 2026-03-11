#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "FootstepData.generated.h"

UCLASS()
class VSLICES_API UFootstepData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// For normal sounds
	UPROPERTY(EditDefaultsOnly, Category="Footsteps")
	TMap<TEnumAsByte<EPhysicalSurface>, USoundBase*> SurfaceSounds;
	UPROPERTY(EditDefaultsOnly, Category="Footsteps")
	USoundBase* DefaultSound = nullptr;
	
	USoundBase* GetSoundForSurface(const EPhysicalSurface Surface) const
	{
		if (USoundBase* Found = SurfaceSounds.FindRef(Surface))
			return Found;
		return DefaultSound;
	}
	// For crouching sounds
	UPROPERTY(EditDefaultsOnly, Category="Footsteps")
	TMap<TEnumAsByte<EPhysicalSurface>, USoundBase*> CrouchSurfaceSounds;
	UPROPERTY(EditDefaultsOnly, Category="Footsteps")
	USoundBase* DefaultCrouchSound = nullptr;

	USoundBase* GetCrouchSoundForSurface(const EPhysicalSurface Surface) const
	{
		if (USoundBase* Found = SurfaceSounds.FindRef(Surface))
			return Found;
		return DefaultCrouchSound;
	}
	
	UPROPERTY(EditDefaultsOnly, Category="Footsteps")
	FName LeftFootBone = TEXT("foot_l");
	UPROPERTY(EditDefaultsOnly, Category="Footsteps")
	FName RightFootBone = TEXT("foot_r");
};