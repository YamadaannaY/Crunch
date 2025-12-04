/*****************  用于将	************************/


#include "Animations/AN_SendTargetGroup.h"
#include "AbilitySystemBlueprintLibrary.h"

void UAN_SendTargetGroup::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return ;

	//1个Socket无法触发Group
	if (TargetSocketName.Num() <=1) return ;

	if (!MeshComp->GetOwner() || !UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner())) return ;

	//用Data存储Socket的位置
	FGameplayEventData Data;
	
	for (int i=1;i<TargetSocketName.Num();++i)
	{
		//需要new，在堆上进行动态分配,因为Add函数内部使用智能指针托管了这个指针，并且会引用计数为0时delete，所以必须在堆上分配，
		//栈指针delete是未定义行为会崩溃，同样不能使用智能指针，这会导致引用计数双重为0释放两次造成崩溃
		FGameplayAbilityTargetData_LocationInfo* LocationInfo=new FGameplayAbilityTargetData_LocationInfo();

		//遍历所有Socket，每两个一组形成一段轨迹，两组轨迹模拟攻击路径
		FVector StratLoc=MeshComp->GetSocketLocation(TargetSocketName[i-1]);
		FVector EndLoc=MeshComp->GetSocketLocation(TargetSocketName[i]);

		//记录起始点和结束点
		LocationInfo->SourceLocation.LiteralTransform.SetLocation(StratLoc);
		LocationInfo->TargetLocation.LiteralTransform.SetLocation(EndLoc);

		/** Adds a new target data to handle, it must have been created with new */
		Data.TargetData.Add(LocationInfo);
	}

	//让EventTag对应的Event获得Socket的SourceLocation和TargetLocation，执行伤害逻辑
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(),EventTag,Data);
	
}
