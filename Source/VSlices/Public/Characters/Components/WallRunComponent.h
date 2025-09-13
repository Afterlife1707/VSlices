#pragma once

#include "CoreMinimal.h"
#include "ParkourComponentBase.h"
#include "Components/ActorComponent.h"
#include "WallRunComponent.generated.h"

UENUM(BlueprintType)
enum class EWallRunDir :uint8
{
	Left, Right, None
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VSLICES_API UWallRunComponent : public UParkourComponentBase
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
	FORCEINLINE float GetCameraTilt() const{return CurrentCameraTilt;}
	FORCEINLINE bool ShouldCameraTilt() const {return bCameraTilt;}
private:
	bool bIsWallRunning;
	bool bCameraTilt;
	float LastWallRunAttempt = 0.0f;
	float WallRunAttemptCooldown = 0.1f;
	bool CheckForWall(const FHitResult& Hit);
	float DefaultGravityScale;
	void UpdateCameraTilt(float DeltaTime);
	float CurrentCameraTilt;
	UPROPERTY()
	AActor* LastWallActor;
	
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Tilt", meta = (ClampMin = "0.0", ClampMax = "45.0"), meta=(AllowPrivateAccess))
	float CameraTiltAngle = 15.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Tilt", meta = (ClampMin = "0.1", ClampMax = "20.0"), meta=(AllowPrivateAccess))
	float CameraTiltSpeed = 5.0f;
	
	FTimerHandle WallRunTimerHandle;
	EWallRunDir Direction = EWallRunDir::None;
};
