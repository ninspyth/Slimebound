// Fill out your copyright notice in the Description page of Project Settings.

/* -----------FEATURES----------
--> Complete Slime Actions -- In Progress 
--> Slime Collision behaviour 
--> Make movement smooth and visually appealing

   -----------TASKS-------------
--> Add acceleration when moving - Done
--> Try Different CameraLag options - Done
--> Add a Dash Action
--> Add a Jump Action 

   -----------BUGS--------------

*/

#include "Slime.h"
#include "UObject/ConstructorHelpers.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "EnhancedInput/Public/EnhancedActionKeyMapping.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput/Public/InputModifiers.h"
#include "Components/SceneComponent.h"

DEFINE_LOG_CATEGORY(LogFail);
DEFINE_LOG_CATEGORY(LogSuccess);

const FVector DFLT_SPRING_ARM_OFFSET = FVector(0.0f, 0.0f, 70.f);

ASlime::ASlime() {
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    InitComponentAndActions();

    bMovementSpeed = 800.0f;
    bMouseSensitivity_X = 100.f;
    bMouseSensitivity_Y = 100.f;

    bSpringArmMaxClamp = 0.0f;
    bSpringArmMinClamp = -80.f;

    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ASlime::InitComponentAndActions() {
    //Root Component
    USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;      

    SlimeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlimeMeshComponent"));
    SlimeMeshComponent->SetupAttachment(RootComponent); 
   
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Fab/Cute_Enemy_-_Slime/cute_enemy_slime.cute_enemy_slime"));
    if(MeshAsset.Succeeded()) {
        SlimeMeshComponent->SetStaticMesh(MeshAsset.Object);

        //Slime defaults
        FVector SlimeScale = FVector(15.0f, 15.0f, 18.0f);
        FRotator SlimeRotate = FRotator({0.0f, 270.0f, 0.0f});

        //rescale the mesh
        SlimeMeshComponent->SetRelativeScale3D(SlimeScale);
        SlimeMeshComponent->SetRelativeRotation(SlimeRotate);

        UE_LOG(LogSuccess, Warning, TEXT("Mesh loaded successfully!"));
    } 
    else {
        UE_LOG(LogFail, Error, TEXT("Error loading glb file"));
    }
   
    // Setup SpringArm - attach to ROOT, not mesh
    SlimeSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SlimeSpringArmComponent"));
    SlimeSpringArmComponent->SetupAttachment(RootComponent);  

    //SpringArm defaults
    SlimeSpringArmComponent->TargetArmLength = 600.0f;
    SlimeSpringArmComponent->bEnableCameraLag = true;
    SlimeSpringArmComponent->CameraLagSpeed = 10.0f;
    SlimeSpringArmComponent->TargetOffset = DFLT_SPRING_ARM_OFFSET;

    FRotator SpringArmRotation = FRotator(-40.0f, 0.0f, 0.0f);
    SlimeSpringArmComponent->SetRelativeRotation(SpringArmRotation);
    
    //Camera Component
    SlimeCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SlimeCameraComponent"));
    SlimeCameraComponent->SetupAttachment(SlimeSpringArmComponent);

    //Move Actions
    Slime_IA_Move = NewObject<UInputAction>();
    Slime_IA_Move->ValueType = EInputActionValueType::Axis2D;

    //Look Action
    Slime_IA_Look = NewObject<UInputAction>();
    Slime_IA_Look->ValueType = EInputActionValueType::Axis2D;

    // Setup Input Mapping Context
    Slime_IMC = NewObject<UInputMappingContext>();

    CheckInitializedAssets();
}

void ASlime::SetupInputMapping() {
    if (!Slime_IMC || !Slime_IA_Move || !Slime_IA_Look) {
        UE_LOG(LogFail, Error, TEXT("Input Actions not initialized!"));
        return;
    }

    if(Slime_IMC->GetMappings().Num() > 0) {
        UE_LOG(LogFail, Error, TEXT("Mappings Already exist"));
        return;
    }

    //Move Action Mapping
    FEnhancedActionKeyMapping& W = Slime_IMC->MapKey(Slime_IA_Move, EKeys::W);
    UInputModifierSwizzleAxis *S_M_W = NewObject<UInputModifierSwizzleAxis>(); 
    S_M_W->Order = EInputAxisSwizzle::YXZ;
    W.Modifiers.Add(S_M_W);

    FEnhancedActionKeyMapping& A = Slime_IMC->MapKey(Slime_IA_Move, EKeys::A);
    UInputModifierNegate *N_M_A = NewObject<UInputModifierNegate>();
    A.Modifiers.Add(N_M_A);

    FEnhancedActionKeyMapping& S = Slime_IMC->MapKey(Slime_IA_Move, EKeys::S);
    UInputModifierNegate *N_M_S = NewObject<UInputModifierNegate>();
    UInputModifierSwizzleAxis *S_M_S = NewObject<UInputModifierSwizzleAxis>(); 
    S_M_S->Order = EInputAxisSwizzle::YXZ;
    S.Modifiers.Add(S_M_S);
    S.Modifiers.Add(N_M_S);

    FEnhancedActionKeyMapping& D = Slime_IMC->MapKey(Slime_IA_Move, EKeys::D);

    //Look Action Mapping    
    FEnhancedActionKeyMapping& Look_Left = Slime_IMC->MapKey(Slime_IA_Look, EKeys::MouseX);
    FEnhancedActionKeyMapping& Look_Right = Slime_IMC->MapKey(Slime_IA_Look, EKeys::MouseY);
    UInputModifierSwizzleAxis *S_Look_Right = NewObject<UInputModifierSwizzleAxis>();
    S_Look_Right->Order = EInputAxisSwizzle::YXZ;
    Look_Right.Modifiers.Add(S_Look_Right);

    //Dash Action Mapping
}

void ASlime::BeginPlay() {
    Super::BeginPlay();

    SetupInputMapping();

    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

    if(Slime_IMC && PlayerController && Subsystem) {
        Subsystem->AddMappingContext(Slime_IMC, 0);
    }
}

void ASlime::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}

void ASlime::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent *Slime_EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);

    if(Slime_EIC) {
        Slime_EIC->BindAction(Slime_IA_Move, ETriggerEvent::Triggered, this, &ASlime::Move);
        Slime_EIC->BindAction(Slime_IA_Look, ETriggerEvent::Triggered, this, &ASlime::Look);
    }
}

void ASlime::Look(const FInputActionValue& Value) {
    const FVector2D LookVector = Value.Get<FVector2D>();

    UE_LOG(LogSuccess, Display, TEXT("LookVector = {%f, %f}"), LookVector.X, LookVector.Y); 

    if(Controller != nullptr && !LookVector.IsZero()) {
        FRotator NewRotation_X = GetActorRotation();

        NewRotation_X += FRotator(0.0f, LookVector.X * bMouseSensitivity_X * GetWorld()->GetDeltaSeconds(), 0.0f);
        AddControllerPitchInput(LookVector.Y * bMouseSensitivity_Y * GetWorld()->GetDeltaSeconds());
        SetActorRotation(NewRotation_X);

        FRotator NewSpringArmRotation = SlimeSpringArmComponent->GetRelativeRotation();
        NewSpringArmRotation.Pitch += LookVector.Y * bMouseSensitivity_Y * GetWorld()->GetDeltaSeconds();
        NewSpringArmRotation.Pitch = FMath::Clamp(NewSpringArmRotation.Pitch, bSpringArmMinClamp, bSpringArmMaxClamp);
        SlimeSpringArmComponent->SetRelativeRotation(NewSpringArmRotation);

        UE_LOG(LogSuccess, Display, TEXT("Spring Arm Rotation: {%f, %f, %f}"), SlimeSpringArmComponent->GetRelativeRotation().Pitch, SlimeSpringArmComponent->GetRelativeRotation().Yaw, SlimeSpringArmComponent->GetRelativeRotation().Roll); 
    }
}

void ASlime::Move(const FInputActionValue& Value) {
    const FVector2D MovementVector = Value.Get<FVector2D>();

    double CurrentSpeed = 0.0f;

    if (Controller != nullptr && !MovementVector.IsZero()) {
        FRotator YawRotation(0, GetActorRotation().Yaw, 0);
        
        FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        FVector Movement = FVector::ZeroVector;
        Movement += ForwardDirection * MovementVector.Y;
        Movement += RightDirection * MovementVector.X;
        Movement += Movement.GetSafeNormal();

        double TargetSpeed = Movement.IsNearlyZero() ? 0.0f : bMovementSpeed;
        
        double NewInterpSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, GetWorld()->GetDeltaSeconds(), 27.0f);

        UE_LOG(LogSuccess, Display, TEXT("NewInterpSpeed = %f, TargetSpeed = %f"), NewInterpSpeed, TargetSpeed); 
        
        FVector NewLocation = GetActorLocation();
        NewLocation += Movement * NewInterpSpeed * GetWorld()->GetDeltaSeconds();

        SetActorLocation(NewLocation);
    }
}

void ASlime::CheckInitializedAssets() {
    if(!SlimeCameraComponent) {
        UE_LOG(LogFail, Error, TEXT("Failed to Create Camera Component"));
    }
    if(!SlimeMeshComponent) {
        UE_LOG(LogFail, Error, TEXT("Failed to Create Mesh Component"));
    }
    if(!Slime_IMC) {
        UE_LOG(LogFail, Error, TEXT("Failed to Create IMC"));
    }
    if(!Slime_IA_Move) {
        UE_LOG(LogFail, Error, TEXT("Failed to Create Input Action -> Move"));
    }
}

