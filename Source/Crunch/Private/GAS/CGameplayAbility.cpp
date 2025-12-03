// Fill out your copyright notice in the Description page of Project Settings.


#include "CGameplayAbility.h"

UAnimInstance* UCGameplayAbility::GetOwnerAnimInstance() const
{
	USkeletalMeshComponent* OwnerSkeletalMeshComp=GetOwningComponentFromActorInfo();
	if (OwnerSkeletalMeshComp)
	{
		return OwnerSkeletalMeshComp->GetAnimInstance();
	}
	
	return nullptr;
}
