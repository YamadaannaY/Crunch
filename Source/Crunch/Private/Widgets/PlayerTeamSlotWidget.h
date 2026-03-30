#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerTeamSlotWidget.generated.h"

class UPA_CharacterDefination;
/**
 * 
 */
UCLASS()
class CRUNCH_API UPlayerTeamSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//当Mouse触发Widget，播放一个放大动画并显示HeroName
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	//当Mouse离开Widget，播放一个缩小动画并显示PlayerName
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	
	//Name为空，Empty为1
	virtual void NativeConstruct() override;
	
	//更新SlotText和SlotIcon
	void UpdateSlot(const FString& PlayerName , const UPA_CharacterDefination* CharacterDefinition);
private:
	UPROPERTY(meta=(BindWidget))
	class UImage* PlayerCharacterIcon;
	
	UPROPERTY(Transient , meta=(BindWidgetAnim))
	UWidgetAnimation* HoverAnim;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* NameText;
	
	FString CachePlayerNameStr;
	FString CacheCharacterNameStr;
	
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	FName CharacterIconMatParamName = "Icon";
	
	UPROPERTY(EditDefaultsOnly,Category="Visual")
	FName CharacterEmptyMatParamName="Empty";
	
	//鼠标悬停的时候修改文本为HeroName，移除时恢复为PlayerName
	void UpdateNameText();
};
