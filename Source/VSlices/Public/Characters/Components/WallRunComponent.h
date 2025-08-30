#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallRunComponent.generated.h"

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
	void StopWallRun() const;
	void Jump();
	FORCEINLINE void ResetWallRun() {bIsWallRunning = false;}
	FORCEINLINE bool IsWallRunning() const {return bIsWallRunning;}
private:
	UPROPERTY()
	class AVSlicesCharacter* OwnerCharacter;
	UPROPERTY()
	class UCharacterMovementComponent* MovementComp;
	bool bIsWallRunning;
	bool CheckForWall(const FHitResult& Hit) const;
	float DefaultGravityScale;
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float WallRunGravityScale = 0.5f;
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float WallRunTimer = 0.75f;
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float JumpForceMultiplier = 1.f;
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float MinWallHeight = 40.f;
	UPROPERTY(EditDefaultsOnly, Category="Wall Run", meta=(AllowPrivateAccess))
	float MinVelocity = 300.f;
	
	FTimerHandle WallRunTimerHandle;
};
