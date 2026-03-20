#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"

UCLASS()
class VSLICES_API APortal : public AActor
{
	GENERATED_BODY()

public:
	APortal();

	UPROPERTY(VisibleAnywhere, Category="Portal")
	UStaticMeshComponent* Frame;
	UPROPERTY(VisibleAnywhere, Category="Portal")
	UStaticMeshComponent* PortalPlane;
	UPROPERTY(VisibleAnywhere, Category="Portal")
	class UArrowComponent* ForwardArrow;
	UPROPERTY(VisibleAnywhere, Category="Portal")
	USceneCaptureComponent2D* PortalCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Portal")
	APortal* LinkedPortal;
	
	UFUNCTION(BlueprintImplementableEvent, Category="Portal")
	void ResetJustTeleported();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
private:
	UPROPERTY()
	class AVSlicesCharacter* Player;
	UPROPERTY()
	UMaterialInstanceDynamic* PortalMat;
	UPROPERTY()
	UTextureRenderTarget2D* PortalRenderTarget;
	UPROPERTY(EditDefaultsOnly, Category="Portal")
	UMaterialInterface* ParentMat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Portal", meta=(AllowPrivateAccess="true"))
	bool bCheckForTeleport;
	UPROPERTY(BlueprintReadWrite, Category="Portal", meta=(AllowPrivateAccess="true"))
	bool bJustTeleportedTo;

	bool bLastInFront;
	FVector LastPosition;
	float CaptureTimer = 0.f;
	UPROPERTY(EditDefaultsOnly, Category="Portal")
	float NearCaptureDistance = 1000.f;
	UPROPERTY(EditDefaultsOnly, Category="Portal")
	float FarCaptureDistance = 2000.f;
	float NearCaptureDistanceSq;
	float FarCaptureDistanceSq;
	
	void UpdateSceneCapture() const;
	void SetClipPlanes() const;
	void CheckForTeleport();
	bool IsPointCrossingPortal(const FVector& Point, const FVector& PortalLocation, const FVector& PortalNormal);
	void Teleport() const;
	FRotator GetNewRotation(const FRotator& InRotation) const;
	FVector UpdateVelocity(const FVector& Velocity) const;
	float GetCaptureInterval() const;
};