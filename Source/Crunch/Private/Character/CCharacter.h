//游戏核心角色类，Hero和Minion都是继承于此，实现共同的基础逻辑

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include  "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "Widgets/RenderActorTargetInterface.h"
#include "CCharacter.generated.h"

struct FOnAttributeChangeData;
enum class ECAbilityInputID : uint8;
class UGameplayAbility;
struct FGameplayEventData;
struct FGameplayTag;

UCLASS()
class ACCharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface,public IRenderActorTargetInterface
{
	GENERATED_BODY()

public:
	ACCharacter();

	//处理了对ASC组件的初始化，并且应用初始化基础属性的函数（仅在客户端初始化）
	void ServerSideInit();

	//只处理对ASC组件的初始化，对属性的初始化交给权威端
	void ClientSideInit();

	//这个函数用在客户端上，判断由客户端控制的玩家角色，因为在客户端上有很多模拟代理，但是只有一个本地Controller，即玩家拥有的Controller
	bool IsLocallyControlledByPlayer();

	//被Controller控制时在服务端调用，此时进行ServerSideInit
	virtual void PossessedBy(AController* NewController) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	//将ASC上的GetAbilities()进一步包装，使得Abilities的获取与ASC解耦
	const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& GetAbilities() const;

	//获取Capture Vector/Rotator
	virtual FVector GetCaptureLocalPosition() const override;
	virtual FRotator GetCaptureLocalRotation() const override;

private:
	UPROPERTY(EditDefaultsOnly,Category="Capture")
	FVector HeadShotCaptureLocalPosition;

	UPROPERTY(EditDefaultsOnly,Category="Capture")
	FRotator HeadShotCaptureLocalRotation;

protected:
	//所有Character在客户端调用此函数是显示OverHeadUI的完美时机
	virtual void BeginPlay() override;

public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/****************************************************/
	/*						GA
	/****************************************************/
	
	//获取ASC
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	//用于客户端本地输入触发Event的情况：将GameplayEvent发送这一行为也传递给服务端，在服务端执行逻辑，触发WaitEvent回调
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendGameplayEventToSelf(const FGameplayTag EventTag, const FGameplayEventData& EventData);
	
	//是否处于聚焦模式，此时禁止移动
	FORCEINLINE bool GetIsInFocusMode() const {return  bIsInFocusMode ; }
	
protected:
	//升级ID对应的GA
	void UpgradeAbilityWithInputID(ECAbilityInputID InputID);
	
private:
	//通过RegisterGameplayTagEvent监听特定Tag的更新并绑定回调函数
	void BindGASChangeDelegates();
	
	//当Tag变化时调用
	void DeadTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void StunTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void AimTagUpdated(const FGameplayTag Tag, int32 NewCount);
	void FocusTagUpdated(const FGameplayTag Tag, int32 NewCount);
	
	//通过GetGameplayAttributeValueChangeDelegate绑定的回调函数
	//当属性修改时调用
	void AccelerationUpdated(const FOnAttributeChangeData& Data);
	void MoveSpeedUpdated(const FOnAttributeChangeData& Data);
	
	//是否处于瞄准状态
	void SetIsAiming(bool bIsAiming);
	
	//不同角色子类（拥有瞄准能力）进行不同的逻辑配置
	virtual void OnAimStatChanged(bool bIsAiming);

	bool bIsInFocusMode = false ;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Gameplay Ability")
	class UCAbilitySystemComponent* CAbilitySystemComponent;
	
	UPROPERTY()
	class UCAttributeSet* CAttributeSet;

	/***********UI************/

	FTimerHandle HeadStatGaugeVisibilityUpdateTimerHandle;
	
	//Gauge可视组件
	UPROPERTY(VisibleAnywhere, Category="UI")
	class UWidgetComponent* OverHeadWidgetComponent;

	//判断距离是否隐藏组件Timer的更新间隔
	UPROPERTY(EditDefaultsOnly, Category="UI")
	float HeadStatGaugeVisibilityUpdateGap = 3.f;

	//要判断距离的平方（以节省开方计算性能为目的）
	UPROPERTY(EditDefaultsOnly, Category="UI")
	float HeadStatGaugeVisibilityRangeSquared = 10000000.f;
	
	//配置Gauge可视组件，将Component转为配置好的GaugeWidget类，并为其绑定委托
	void ConfigureOverHeadStatusWidget();

	//客户端调用，Timer绑定的回调，对客户端中的每一个此角色实例调用，根据与本地客户端角色实例的距离判断是否要显示自己的OverheadUI是否显示
	void UpdateHeadGaugeVisibility() const ;

	//Death状态下在客户端调用，判断是否显示OverHeadWidget
	void SetStatusGaugeEnabled(bool bEnabled);

	/************************** Death and Respawn ********************************/
public:
	//通过DeadTag判断DeadStat
	bool IsDead() const;

	//直接移除DeadTag
	void ReSpawnImmediative() const ;

private:
	//Mesh相对Capsule的变换，用于换Mesh，RagDoll复位等情景下保持新的Mesh变换与最开始相同
	FTransform MeshRelativeTransform;

	UPROPERTY(EditDefaultsOnly, Category="Death")
	UAnimMontage* DeathMontage;

	//如果在Montage完成的一瞬间就改变其Physics可能会造成不自然卡顿，因此加一个缓冲值
	UPROPERTY(EditDefaultsOnly, Category="Death")
	float DeathMontageFinishTimeShift = -0.8f;
	FTimerHandle DeathMontageTImerHandle;

	//当DeathMontage播放完毕时调用，进入RagDoll状态
	void DeathMontageFinished();

	//RagDoll实现
	void SetRagDollEnabled(bool bEnabled) const ;

	//播放DeathMontage并设置一个定时器调用DeathMontageFinished
	void PlayDeathAnimation();

	//当DeadTag生效时的触发函数
	void StartDeathSequence();

	//当DeadTag到期移除时的函数
	void Respawn();

	//Dead / Respawn 函数调用时进行的逻辑，根据不同的Character子类执行不同的逻辑
	virtual void OnDead();
	virtual void OnRespawn();

	/******************************* Team ***********************************/
public:
	/** Assigns Team Agent to given TeamID 在Controller OnPossess时调用*/
	//Problem:为什么不可以在这里调用PickSkin函数，根据ID换Mesh?
	//Answer:这个函数只在服务端执行，如果在服务端调用，ID会复制给所有客户端，但是Mesh默认是不复制的，也就是不会复制给客户端。
	//换句话说，PickSkin确实执行了，但是客户端看不到，应该用OnRep，让客户端收到ID的那一刻根据ID改变Mesh
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const override;

private:
	//虽然已经在Controller上设置了，但是Character本身也需要设置，用来在其他客户端端口让其他的Character能够辨别TeamID
	//这样设计是因为Controller唯一，但是Character是可以改变的，所以用Controller上的ID作为在权威端的赋值，而Character
	//本身并不决定ID，只负责将ID带在身上让其他客户端进行辨别
	UPROPERTY(ReplicatedUsing="OnRep_TeamID")
	FGenericTeamId TeamID;

	UFUNCTION()
	virtual void OnRep_TeamID();

	/****************************** AI ******************************/

	//在Game.ini中我们手动关闭了bAutoRegisterAllPawnAsSource，目的是为了自己配置感知刺激源，让AI在某些情况无法感知Pawn。

	//设置AI感知刺激源，如果true则将角色注册为AI可以感知的目标，如果false则取消注册，让AI无法感知。这个函数用于在角色Dead和ReSpawn中
	//实现在角色拥有DeadTag的情况下让AI无法感知
	void SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled) const ;

	UPROPERTY()
	class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;

	/****************************** Stun ******************************/
private:
	UPROPERTY(EditDefaultsOnly, Category="Stun")
	UAnimMontage* StunMontage;

	//RegisterTagEvent对StunTag的回调
	virtual void OnStun();
	virtual void OnRecoveryFromStun();
};
