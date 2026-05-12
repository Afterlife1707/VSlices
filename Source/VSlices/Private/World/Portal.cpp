// Portal.cpp
#include "World/Portal.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/CameraComponent.h"
#include "Characters/VSlicesCharacter.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "LoggingMacros.h"

APortal::APortal()
{
    PrimaryActorTick.bCanEverTick = true;

    Frame = CreateDefaultSubobject<UStaticMeshComponent>("Frame");
    SetRootComponent(Frame);
    PortalPlane = CreateDefaultSubobject<UStaticMeshComponent>("PortalPlane");
    PortalPlane->SetupAttachment(Frame);
    ForwardArrow = CreateDefaultSubobject<UArrowComponent>("ForwardArrow");
    ForwardArrow->SetupAttachment(Frame);
    PortalCamera = CreateDefaultSubobject<USceneCaptureComponent2D>("PortalCamera");
    PortalCamera->SetupAttachment(Frame);
}

void APortal::BeginPlay()
{
    Super::BeginPlay();
    
    Player = Cast<AVSlicesCharacter>(GetWorld()->GetFirstPlayerController()->GetCharacter());
    if (!Player)
    {
        LOG_ERROR("Missing Player!");
        return;
    }
    Player->GetFollowCamera()->FieldOfView = PortalCamera->FOVAngle;
    
    SetTickGroup(TG_PostUpdateWork);
    
    if (!ParentMat)
    {
        LOG_ERROR("Missing Parent Mat! Assign in Portal Class Defaults");
        return;
    }
    PortalMat = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), ParentMat);
    PortalPlane->SetMaterial(0, PortalMat);
    
    NearCaptureDistanceSq = NearCaptureDistance * NearCaptureDistance;
    FarCaptureDistanceSq  = FarCaptureDistance  * FarCaptureDistance;
    const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());
    constexpr float RTScale = 0.75f;
    PortalRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(),FMath::RoundToInt(ViewportSize.X * RTScale),FMath::RoundToInt(ViewportSize.Y * RTScale));
    PortalMat->SetTextureParameterValue("Texture", PortalRenderTarget);
    LinkedPortal->PortalCamera->TextureTarget = PortalRenderTarget;
    
    PortalCamera->bCaptureEveryFrame = false; 
    PortalCamera->bCaptureOnMovement = false;
    PortalCamera->DetailMode = DM_Low;
    PortalCamera->ShowFlags.SetAmbientOcclusion(false);
    PortalCamera->ShowFlags.SetMotionBlur(false);
    PortalCamera->ShowFlags.SetBloom(false);
    PortalCamera->ShowFlags.SetLensFlares(false);
    PortalCamera->ShowFlags.Cloud = false;
    PortalCamera->ShowFlags.SetVolumetricFog(false);
    
    SetClipPlanes();
}

void APortal::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Early out if player is beyond far distance entirely
    const float DistSq = FVector::DistSquared(Player->GetActorLocation(), GetActorLocation());
    if (DistSq > FarCaptureDistanceSq * 4.f) // 2× the far distance
    {
        if (!bJustTeleportedTo && bCheckForTeleport)
            CheckForTeleport();
        return;
    }

    if (PortalPlane->WasRecentlyRendered(0.1f))
    {
        CaptureTimer += DeltaTime;
        if (CaptureTimer >= GetCaptureInterval())
        {
            UpdateSceneCapture();
            CaptureTimer = 0.f;
        }
    }

    if (!bJustTeleportedTo && bCheckForTeleport)
        CheckForTeleport();
}

void APortal::UpdateSceneCapture() const
{
    const USceneComponent* CamTransform = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetTransformComponent();

    FVector LocalPos = GetActorTransform().InverseTransformPosition(CamTransform->GetComponentLocation());
    LocalPos.X *= -1;
    LocalPos.Y *= -1;
    const FVector NewLocation = LinkedPortal->GetActorTransform().TransformPosition(LocalPos);

    FRotator LocalRot = GetActorTransform().InverseTransformRotation(CamTransform->GetComponentQuat()).Rotator();
    LocalRot.Yaw += 180.f;
    const FRotator NewRotation = LinkedPortal->GetActorTransform().TransformRotation(LocalRot.Quaternion()).Rotator();

    LinkedPortal->PortalCamera->SetWorldLocationAndRotation(NewLocation, NewRotation);
    LinkedPortal->PortalCamera->CaptureScene();
}

void APortal::SetClipPlanes() const
{
    if (!LinkedPortal) return;
    PortalCamera->bEnableClipPlane = true;
    PortalCamera->ClipPlaneBase = PortalPlane->GetComponentLocation() + ForwardArrow->GetForwardVector() * -3;
    PortalCamera->ClipPlaneNormal = ForwardArrow->GetForwardVector();
}

void APortal::CheckForTeleport()
{
    if (Player)
    {
        if (IsPointCrossingPortal(Player->GetFollowCamera()->GetComponentLocation(), GetActorLocation(), ForwardArrow->GetForwardVector()))
            Teleport();
    }
}

bool APortal::IsPointCrossingPortal(const FVector& Point, const FVector& PortalLocation, const FVector& PortalNormal)
{
    const bool bIsInFront = FVector::DotProduct(Point - PortalLocation, PortalNormal) >= 0;
    const FPlane Plane = UKismetMathLibrary::MakePlaneFromPointAndNormal(PortalLocation, PortalNormal);
    float T; FVector Intersection;
    const bool bIsIntersect = UKismetMathLibrary::LinePlaneIntersection(LastPosition, Point, Plane, T, Intersection);
    const bool bIsCrossing = bIsIntersect && !bIsInFront && bLastInFront;
    bLastInFront = bIsInFront;
    LastPosition = Point;
    return bIsCrossing;
}

void APortal::Teleport() const
{
    if (!Player) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    LinkedPortal->bJustTeleportedTo = true;

    FVector LocalPos = GetActorTransform().InverseTransformPosition(Player->GetActorLocation());
    LocalPos.X *= -1;
    LocalPos.Y *= -1;
    const FVector NewLocation = LinkedPortal->GetActorTransform().TransformPosition(LocalPos);

    FRotator NewActorRot = GetNewRotation(Player->GetActorRotation());
    NewActorRot.Normalize();
    FRotator NewControlRot = GetNewRotation(PC->GetControlRotation());
    NewControlRot.Normalize();
    Player->SetActorTransform(FTransform(NewActorRot, NewLocation), false, nullptr, ETeleportType::TeleportPhysics);
    PC->SetControlRotation(NewControlRot);

    if (Player)
    {
        TWeakObjectPtr WeakPlayer = Player;
        FTimerHandle OrientTimerHandle;
        GetWorldTimerManager().SetTimer(OrientTimerHandle, [WeakPlayer]()
        {
            if (WeakPlayer.IsValid())
            {
                WeakPlayer->SetOrient(true);
            }
        }, 0.2f, false);
    }

    if (UPawnMovementComponent* MoveComp = Player->GetMovementComponent())
        MoveComp->Velocity = UpdateVelocity(MoveComp->Velocity);

    UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->SetGameCameraCutThisFrame();
    PortalCamera->bCameraCutThisFrame = true;

    LinkedPortal->ResetJustTeleported();
}

FRotator APortal::GetNewRotation(const FRotator& InRotation) const
{
    FVector X, Y, Z;
    UKismetMathLibrary::GetAxes(InRotation, X, Y, Z);

    auto TransformAxis = [&](FVector V) -> FVector
    {
        V = GetActorTransform().InverseTransformVectorNoScale(V);
        V.X *= -1; V.Y *= -1;
        return LinkedPortal->GetActorTransform().TransformVectorNoScale(V);
    };

    return UKismetMathLibrary::MakeRotationFromAxes(TransformAxis(X), TransformAxis(Y), TransformAxis(Z));
}

FVector APortal::UpdateVelocity(const FVector& Velocity) const
{
    FVector LocalDir = GetActorTransform().InverseTransformVector(Velocity.GetSafeNormal());
    LocalDir.X *= -1;
    LocalDir.Y *= -1;
    return LinkedPortal->GetActorTransform().TransformVector(LocalDir) * Velocity.Size();
}

float APortal::GetCaptureInterval() const
{
    if (!Player) return 0.f;

    const float DistSq = FVector::DistSquared(Player->GetActorLocation(), GetActorLocation());
    if (DistSq > FarCaptureDistanceSq)  return 0.1f;
    if (DistSq > NearCaptureDistanceSq) return 0.05f;
    return 0; 
}