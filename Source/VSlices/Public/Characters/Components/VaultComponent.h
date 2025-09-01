#pragma once

#include "CoreMinimal.h"
#include "ParkourComponentBase.h"
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

USTRUCT()
struct FVaultableObstacle
{
	GENERATED_BODY()
	
	FVector TopLocation;
	FVector Normal;
	float Height;
	bool bIsWall;        // true = wall to vault over, false = platform to mantle
	bool bIsThick;       // true = climb, false = vault
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VSLICES_API UVaultComponent : public UParkourComponentBase
{
	GENERATED_BODY()
	struct FTraceResult
	{
		FVector Location;
		FVector Normal;
	};
public:
	UVaultComponent();

	bool TryVault(const bool bWasSprinting);
	bool IsVaulting() const { return bIsVaulting; }

	UPROPERTY(EditDefaultsOnly, Category="Vaulting|Animations")
	UAnimMontage* VaultShortMontage;
	UPROPERTY(EditDefaultsOnly, Category="Vaulting|Animations")
	UAnimMontage* VaultTallMontage;
	UPROPERTY(EditDefaultsOnly, Category="Vaulting|Animations")
	UAnimMontage* ClimbShortMontage;
	UPROPERTY(EditDefaultsOnly, Category="Vaulting|Animations")
	UAnimMontage* ClimbTallMontage;
	EVaultType CurrentVaultType;

	void FinishVault();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditAnywhere, Category="Vault|Tracing")
	float MinTraceHeight = -40.0f;
	UPROPERTY(EditAnywhere, Category="Vault|Tracing")
	float MaxTraceHeight = 180.0f;
	//vault
	UPROPERTY(EditAnywhere, Category="Vault")
	float TraceDistance = 120.f;
	UPROPERTY(EditAnywhere, Category="Vault")
	float MinHeightForShortVault = 2.f;
	UPROPERTY(EditAnywhere, Category="Vault")
	float MaxHeightForShortVault = 50.f;
	UPROPERTY(EditAnywhere, Category="Vault")
	float MaxHeightForTraverse = 216.f;
	UPROPERTY(EditAnywhere, Category="Vault")
	float ThicknessForClimb = 60.f;
	
private:
	FVector VaultStartLocation;
	FVector VaultTargetLocation;
	FRotator VaultStartRotation;
	FRotator VaultTargetRotation;
	float VaultLerpAlpha = 0.f;
	float VaultLerpTime = 0.8f;
	float VaultArcPeak;
	FTimerHandle VaultMoveTimerHandle;

	//EVaultType VaultType;
	bool bIsVaulting = false;
	bool PerformTrace(FHitResult& OutHit, const FVector& Start, const FVector& End) const;

	//cache
	FCollisionQueryParams TraceParams;
	FCollisionObjectQueryParams ObjectParams;
	float CapsuleRadius = 0.f;
	float CapsuleHalfHeight = 0.f;
    
	bool FindVaultableObstacle(FVaultableObstacle& OutObstacle, bool bWasSprinting) const;
	bool AnalyzeObstacle(const FHitResult& Hit, FVaultableObstacle& OutObstacle) const;
	bool FindWallTop(const FHitResult& WallHit, FVector& OutWallTop, float& OutHeight) const;
	bool ValidateLandingSpace(const FVector& ObstacleTop) const;
	bool ExecuteVault(const FVaultableObstacle& Obstacle);
	void StartVault(const EVaultType VaultType, const FVector& TargetLocation, const FRotator& TargetRotation);
	
	void UpdateVaultMotion(const float DeltaTime) const;
	void UpdateClimbMotion(const float DeltaTime) const;
	float CalculateArcOffset() const;
	
	bool IsObstacleThick(const FHitResult& Hit, const FVector& WallTop) const;
	
	UAnimMontage* GetVaultMontage(const EVaultType VaultType) const;
	void DrawTraceDebug(const FVector& Start, const FVector& End, const bool bHit, const FVector& HitLocation) const;
};
