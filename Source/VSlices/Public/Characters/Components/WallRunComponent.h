#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallRunComponent.generated.h"

UENUM(BlueprintType)
enum class EWallRunDir :uint8
{
	Left, Right, None
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VSLICES_API UWallRunComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWallRunComponent();
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void TryWallRun(const FHitResult& Hit);
	void StartWallRun(const FVector& WallNormal);
	void StopWallRun();
	void Jump();
	void ResetWallRun();
	UFUNCTION(BlueprintCallable, Category="Wall Run")
	FORCEINLINE bool IsWallRunning() const {return bIsWallRunning;}
	UFUNCTION(BlueprintCallable, Category="Wall Run")
	FORCEINLINE EWallRunDir GetWallRunDirection() const{return Direction;}
private:
	UPROPERTY()
	class AVSlicesCharacter* OwnerCharacter;
	UPROPERTY()
	class UCharacterMovementComponent* MovementComp;
	bool bIsWallRunning;
	float LastWallRunAttempt = 0.0f;
	float WallRunAttemptCooldown = 0.1f;
	bool CheckForWall(const FHitResult& Hit);
	float DefaultGravityScale;
	
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float WallRunGravityScale = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float WallRunTimer = 0.75f;
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float JumpForceMultiplier = 1.f;
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float JumpHeightBoost = 100.f;
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float MinWallHeight = 40.f;
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float MinWallAngleDot = 0.6f;
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float MinVelocity = 300.f;
	
	FTimerHandle WallRunTimerHandle;
	EWallRunDir Direction = EWallRunDir::None;
};
