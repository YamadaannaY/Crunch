// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


/*
 *	角色组件：处理ShopWidget相关的逻辑
 */

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UPA_ShopItem;
class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRUNCH_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventoryComponent();

	//监听委托的回调函数，判断完Item有效性后调用Server_Purchase
	void TryPurchase(const UPA_ShopItem* ItemToPurchase);

	//获取GoldAttribute值
	float GetGold() const ;
protected:
	//获取ASC
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UAbilitySystemComponent* OwnerASC;

	/******************* Server **********************/

	//购买逻辑应该在服务端执行
	UFUNCTION(Server, Reliable,WithValidation)
	void Server_Purchase(const UPA_ShopItem* ItemToPurchase);
};
