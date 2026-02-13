// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Slime.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFail, Error, All);
DECLARE_LOG_CATEGORY_EXTERN(LogSuccess, Display, All);

UCLASS()
class SLIMEBOUND_API ASlime : public APawn {
    GENERATED_BODY()

public:
    // Sets default values for this pawn's properties
    ASlime();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:	
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Called to bind functionality to input
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
    UPROPERTY(VisibleAnywhere, Category = Camera)
    USpringArmComponent *SlimeSpringArmComponent;

    UPROPERTY(VisibleAnywhere, Category = Camera)
    UCameraComponent *SlimeCameraComponent;

    UPROPERTY(VisibleAnywhere, Category = Mesh)
    UStaticMeshComponent *SlimeMeshComponent;

    //Input Mapping Context 
    UPROPERTY(EditAnywhere, Category = Input)
    UInputMappingContext *Slime_IMC;

    //Input Actions
    UPROPERTY(EditAnywhere, Category = Action)    
    UInputAction *Slime_IA_Move;

    UPROPERTY(EditAnywhere, Category = Action)
    UInputAction *Slime_IA_Dash;

    UPROPERTY(EditAnywhere, Category = Action)
    UInputAction *Slime_IA_Look;

    UPROPERTY(VisibleAnywhere, Category = Property) 
    float bMovementSpeed;

    UPROPERTY(VisibleAnywhere, Category = Property) 
    float bMouseSensitivity_X;

    UPROPERTY(VisibleAnywhere, Category = Property) 
    float bMouseSensitivity_Y;

    UPROPERTY(VisibleAnywhere, Category = Property) 
    float bSpringArmMinClamp;

    UPROPERTY(VisibleAnywhere, Category = Property) 
    float bSpringArmMaxClamp;

    //Move Function
    UFUNCTION()
    void Move(const FInputActionValue& Value);

    //Look Function
    UFUNCTION()
    void Look(const FInputActionValue& Value);

    // UFUNCTION()
    void Dash(const FInputActionValue& Value);

    UFUNCTION()
    void SetupInputMapping();
    
    UFUNCTION()
    void InitComponentAndActions();

    UFUNCTION()
    void CheckInitializedAssets();

};
