#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_Footstep.generated.h"

UENUM(BlueprintType)
enum class EFoot : uint8
{
	Left,
	Right
};

UCLASS()
class VSLICES_API UAN_Footstep : public UAnimNotify
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Footstep")
	EFoot Foot = EFoot::Left;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};