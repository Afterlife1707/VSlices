#include "Animations/AN_Footstep.h"

#include "Characters/VSlicesCharacter.h"

void UAN_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp) return;
	AVSlicesCharacter* Character = Cast<AVSlicesCharacter>(MeshComp->GetOwner());
	if (!Character) return;
	Character->OnFootstep(Foot);
}