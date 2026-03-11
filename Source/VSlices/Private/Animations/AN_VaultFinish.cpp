#include "Animations/AN_VaultFinish.h"

#include "Characters/Components/VaultComponent.h"
#include "Characters/VSlicesCharacter.h"

void UAN_VaultFinish::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (MeshComp)
	{
		AActor* Owner = MeshComp->GetOwner();
		if (const AVSlicesCharacter* Character = Cast<AVSlicesCharacter>(Owner))
		{
			if (UVaultComponent* VaultComp = Character->GetVaultComponent())
			{
				VaultComp->FinishVault();
			}
		}
	}
}
