#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VaultComponent.generated.h"

UENUM(BlueprintType)
enum class EVaultType : uint8
{
	VAULT_SHORT UMETA(DisplayName = "Short Vault"),
	VAULT_TALL UMETA(DisplayName = "Tall Vault"),
	CLIMB_SHORT UMETA(DisplayName = "Short Climb"),
	CLIMB_TALL UMETA(DisplayName = "Tall Climb")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VSLICES_API UVaultComponent : public UActorComponent
{
	GENERATED_BODY()

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

	void StartVault(const EVaultType VaultType, const FVector& TargetLocation);
	void MoveCharacterToTarget();
	//vault
	UPROPERTY(EditAnywhere, Category="Vaulting|Settings")
	float MinHeightForShortVault = 20.f;
	UPROPERTY(EditAnywhere, Category="Vaulting|Settings")
	float MaxHeightForShortVault = 50.f;
	UPROPERTY(EditAnywhere, Category="Vaulting|Settings")
	float MaxHeightForTraverse = 120.f;
	UPROPERTY(EditAnywhere, Category="Vaulting|Settings")
	float MaxVaultThickness = 60.f;
	
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
};
