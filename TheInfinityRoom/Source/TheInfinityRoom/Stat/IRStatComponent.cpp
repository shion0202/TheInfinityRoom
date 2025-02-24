// Fill out your copyright notice in the Description page of Project Settings.

#include "IRStatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameData/IRGameSingleton.h"

UIRStatComponent::UIRStatComponent()
{
	bWantsInitializeComponent = true;

	Level = 1;
}

void UIRStatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SetLevel(Level);
	SetHp(GetTotalStat().MaxHp);
}

void UIRStatComponent::SetLevel(int32 NewLevel)
{
	Level = FMath::Clamp(NewLevel, 1, UIRGameSingleton::Get().CharacterMaxLevel);
	SetBaseStat(UIRGameSingleton::Get().GetCharacterStat(NewLevel));
}

float UIRStatComponent::ApplyDamage(float InDamage)
{
	const float PrevHp = CurrentHp;
	const float ActualDamage = FMath::Clamp(InDamage, 0.f, InDamage);

	SetHp(PrevHp - ActualDamage);
	if (CurrentHp < KINDA_SMALL_NUMBER)
	{
		OnHpZero.Broadcast();
	}

	return CurrentHp;
}

void UIRStatComponent::SetBaseStat(const FIRCharacterStat& InBaseStat)
{
	BaseStat = InBaseStat;
	CalculateTotalStat();
}

void UIRStatComponent::SetWeaponStat(const FIRCharacterStat& InWeaponStat)
{
	WeaponStat = InWeaponStat;
	CalculateTotalStat();
}

void UIRStatComponent::AddScrollStat(const FIRCharacterStat& InScrollStat)
{
	ScrollStats.Add(InScrollStat);
	CalculateTotalStat();

	float HpValue = InScrollStat.MaxHp;
	if (HpValue > 0)
	{
		int32 NewHp = (int32)HpValue * 0.5f;
		HealHpValue((float)NewHp);
	}
}

void UIRStatComponent::CalculateTotalStat()
{
	FIRCharacterStat NewStat = BaseStat + WeaponStat;
	for (const FIRCharacterStat& ScrollStat : ScrollStats)
	{
		NewStat = NewStat + ScrollStat;
	}

	FIRCharacterStat CharacterMaxStat = UIRGameSingleton::Get().GetCharacterMaxStat();
	for (TFieldIterator<FNumericProperty> PropIt(FIRCharacterStat::StaticStruct()); PropIt; ++PropIt)
	{
		float CurrentStat = 0.f;
		PropIt->GetValue_InContainer((const void*)&NewStat, &CurrentStat);
		float MaxStat = 0.f;
		PropIt->GetValue_InContainer((const void*)&CharacterMaxStat, &MaxStat);

		if (CurrentStat > MaxStat)
		{
			PropIt->SetValue_InContainer((void*)&NewStat, &MaxStat);
		}
	}

	TotalStat = NewStat;
	OnStatChanged.Broadcast(TotalStat);

	if (CurrentHp > TotalStat.MaxHp)
	{
		SetHp(TotalStat.MaxHp);
	}
	else
	{
		OnHpChanged.Broadcast(TotalStat.MaxHp, CurrentHp);
	}
}

void UIRStatComponent::HealHpRatio(float InHealRatio)
{
	int32 HealAmount = (int32)TotalStat.MaxHp * InHealRatio * 0.01;
	SetHp(CurrentHp + (float)HealAmount);
}

void UIRStatComponent::HealHpValue(float InHealValue)
{
	SetHp(CurrentHp + InHealValue);
}

void UIRStatComponent::SetHp(float NewHp)
{
	CurrentHp = FMath::Clamp(NewHp, 0.f, TotalStat.MaxHp);
	OnHpChanged.Broadcast(TotalStat.MaxHp, CurrentHp);
}
