#include "Animations/VaultFinishNotify.h"
#include "Characters/Components/VaultComponent.h"
#include "Characters/VSlicesCharacter.h"

void UVaultFinishNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
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
