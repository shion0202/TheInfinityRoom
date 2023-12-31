// Fill out your copyright notice in the Description page of Project Settings.


#include "IRCharacterPlayer.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

AIRCharacterPlayer::AIRCharacterPlayer()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SPRINGARM"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 500.f;
	SpringArm->SetRelativeRotation(FRotator(-35.f, 0.f, 0.f));
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CAMERA"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC(
		TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Input/IMC_Default.IMC_Default'"));
	if (IMC.Object)
	{
		DefaultMappingContext = IMC.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_JUMP(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Jump.IA_Jump'"));
	if (IA_JUMP.Object)
	{
		JumpAction = IA_JUMP.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_MOVE(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Move.IA_Move'"));
	if (IA_MOVE.Object)
	{
		MoveAction = IA_MOVE.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_LOOK(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Look.IA_Look'"));
	if (IA_LOOK.Object)
	{
		LookAction = IA_LOOK.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_ATTACK(
		TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Attack.IA_Attack'"));
	if (IA_ATTACK.Object)
	{
		AttackAction = IA_ATTACK.Object;
	}
}

void AIRCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = CastChecked<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
				PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AIRCharacterPlayer::Tick(float DeltaTime)
{
}

void AIRCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent =
		CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AIRCharacterPlayer::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIRCharacterPlayer::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIRCharacterPlayer::Look);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AIRCharacterPlayer::Attack);
	}
}

void AIRCharacterPlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void AIRCharacterPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AIRCharacterPlayer::Jump()
{
	GetMesh()->GetAnimInstance()->Montage_Stop(0.f, AttackMontage);

	bPressedJump = true;
	JumpKeyHoldTime = 0.0f;

	CurrentCombo = 0;
	HasNextComboCommand = false;
}

void AIRCharacterPlayer::Attack()
{
	ProcessAttack();
}
