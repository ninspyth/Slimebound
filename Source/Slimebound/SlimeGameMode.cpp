// Fill out your copyright notice in the Description page of Project Settings.


#include "SlimeGameMode.h"
#include "Slime.h"

ASlimeGameMode::ASlimeGameMode() {
    DefaultPawnClass = ASlime::StaticClass();
}
