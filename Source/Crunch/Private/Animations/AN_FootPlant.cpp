#include "Animations/AN_FootPlant.h"
#include "Sandbox/SandboxAnimInstance.h"

void UAN_FootPlant::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	USandboxAnimInstance* AnimInst = Cast<USandboxAnimInstance>(MeshComp->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->OnFootPlantNotify(Foot);
	}
}

FString UAN_FootPlant::GetNotifyName_Implementation() const
{
	return Foot == EFootPlant::Left ? TEXT("Foot_L") : TEXT("Foot_R");
}
