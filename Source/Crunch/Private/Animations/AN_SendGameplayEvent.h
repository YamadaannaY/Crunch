//在Montage中配置此Notify可以将一个带有Tag的GameplayEvent发送给Actor，让监听此Tag的Event触发回调

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_SendGameplayEvent.generated.h"

/**
 * 
 */

UCLASS()
class UAN_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp,UAnimSequenceBase* Animation,const FAnimNotifyEventReference& EventReference) override;
private:
	UPROPERTY(EditAnywhere,Category="Gameplay Ability")
	FGameplayTag EventTag;

	//将Notify的函数名变为Tag名
	virtual FString GetNotifyName_Implementation() const override;
};
