// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include  "AbilitySystemInterface.h"
#include "CCharacter.generated.h"

struct FGameplayTag;

UCLASS()
class ACCharacter : public ACharacter,public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACCharacter();

	//服务端的Init,处理了对ASC的初始化，并且应用初始化基础属性的函数（仅在客户端初始化）
	void ServerSideInit();
	//客户端的Init，只处理对ASC的初始化，对属性的初始化交给权威端
	void ClientSideInit();

	//这个函数用在客户端上，判断由客户端控制的玩家角色，因为在客户端上只有一个Controller，即玩家拥有的Controller，从而判断真正的玩家是哪一个
	bool IsLocallyControlledByPlayer();

	//用于AIController控制CCharacter时也能进行ServerInit
	virtual void PossessedBy(AController* NewController) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/****************************************************/
	/*						GA
	/****************************************************/
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
private:

	//通过RegisterGameplayTagEvent监听特定Tag的更新并绑定回调函数
	void BindGASChangeDelegates();

	//当DeadTag（附加或者删除时）实现的函数
	void DeadTagUpdated(const FGameplayTag Tag,int32 NewCount);
	
	UPROPERTY(VisibleDefaultsOnly,Category="Gameplay Ability")
	class UCAbilitySystemComponent* CAbilitySystemComponent;
	UPROPERTY()
	class UCAttributeSet* CAttributeSet;

	/***********UI************/

	//Gauge可视组件
	UPROPERTY(VisibleAnywhere,Category="UI")
	class UWidgetComponent* OverHeadWidgetComponent;

	//判断距离是否隐藏组件Timer的更新间隔
	UPROPERTY(EditDefaultsOnly,Category="UI")
	float  HeadStatGaugeVisibilityUpdateGap=3.f;

	//要判断距离的平方（以节省开方计算性能为目的）
	UPROPERTY(EditDefaultsOnly,Category="UI")
	float HeadStatGaugeVisibilityRangeSquared=10000000.f;

	FTimerHandle HeadStatGaugeVisibilityUpdateTimerHandle;

	//配置Gauge可视组件，将Component转为配置好的GaugeWidget类，并为其绑定委托
	void ConfigureOverHeadStatusWidget();

	//Timer绑定的回调，对客户端中的每一个角色类调用根据距离判断是否显示自己的OverheadUI是否显示
	void  UpdateHeadGaugeVisibility();

	void SetStatusGaugeEnabled(bool bEnabled);

	/*********** Death and Respawn ************/
	FTransform MeshRelativeTransform;

	UPROPERTY(EditDefaultsOnly,Category="Death")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly,Category="Death")
	float DeathMontageFinishTimeShift=-0.8f;
	FTimerHandle DeathMontageTImerHandle;

	void DeathMontageFinished();
	void SetRagDollEnabled(bool bEnabled);

	void PlayDeathAnimation();

	//当DeadTag生效时的触发函数
	void StartDeathSequence();

	//当DeadTag到期移除时的函数
	void Respawn();

	virtual void OnDead();
	virtual void OnRespawn();

};
