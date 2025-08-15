#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VaultComponent.generated.h"

UENUM(BlueprintType)
enum class EVaultType : uint8
{
	Vault_Short UMETA(DisplayName = "Short Vault"),
	Vault_Tall UMETA(DisplayName = "Tall Vault"),
	Climb_Short UMETA(DisplayName = "Short Climb"),
	Climb_Tall UMETA(DisplayName = "Tall Climb")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VSLICES_API UVaultComponent : public UActorComponent
{
	GENERATED_BODY()
	struct FTraceResult
	{
		FVector Location;
		FVector Normal;
	};
public:
	UVaultComponent();

	bool TryVault();
	bool IsVaulting() const { return bIsVaulting; }

	UPROPERTY(EditDefaultsOnly, Category="Vaulting|Animations")
	UAnimMontage* VaultShortMontage;
	UPROPERTY(EditDefaultsOnly, Category="Vaulting|Animations")
	UAnimMontage* VaultTallMontage;
	UPROPERTY(EditDefaultsOnly, Category="Vaulting|Animations")
	UAnimMontage* ClimbShortMontage;
	UPROPERTY(EditDefaultsOnly, Category="Vaulting|Animations")
	UAnimMontage* ClimbTallMontage;
	
	void FinishVault();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	bool ForwardTrace(FTraceResult& OutHitResult
	) const;
	bool HeightTrace(const FTraceResult& ForwardHit, FVector& OutWallTop, float& OutWallHeight) const;
	bool ThicknessTrace(const FTraceResult& ForwardHit, const FVector& WallTop) const;
	void StartVault(const EVaultType VaultType, const FVector& TargetLocation, const FRotator& TargetRotation);
	
	//vault
	UPROPERTY(EditAnywhere, Category="Vaulting|Settings")
	float MinHeightForShortVault = 20.f;
	UPROPERTY(EditAnywhere, Category="Vaulting|Settings")
	float MaxHeightForShortVault = 50.f;
	UPROPERTY(EditAnywhere, Category="Vaulting|Settings")
	float MaxHeightForTraverse = 120.f;
	UPROPERTY(EditAnywhere, Category="Vaulting|Settings")
	float ThicknessForClimb = 60.f;
private:
	UPROPERTY()
	class AVSlicesCharacter* OwnerCharacter;
	UPROPERTY()
	class UCharacterMovementComponent* MovementComp;
	
	FVector VaultStartLocation;
	FVector VaultTargetLocation;
	FRotator VaultStartRotation;
	FRotator VaultTargetRotation;
	float VaultLerpAlpha = 0.f;
	float VaultLerpTime = 0.8f;

	FTimerHandle VaultMoveTimerHandle;

	//EVaultType VaultType;
	bool bIsVaulting = false;
	bool PerformTrace(FHitResult& OutHit, const FVector& Start, const FVector& End) const;
	UPROPERTY()
	float VaultArcPeak;
};
