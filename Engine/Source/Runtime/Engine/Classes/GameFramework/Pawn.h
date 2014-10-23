// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "AI/Navigation/NavAgentInterface.h"
#include "GameFramework/Actor.h"
#include "Pawn.generated.h"

ENGINE_API DECLARE_LOG_CATEGORY_EXTERN(LogDamage, Warning, All);

class AController;
class APhysicsVolume;
class APlayerController;
class APlayerState;
class UCanvas;
class UDamageType;
class UInputComponent;
class UNetConnection;
class UPawnMovementComponent;
class UPawnNoiseEmitterComponent;
class UPlayer;
class UPrimitiveComponent;

/** 
 * Pawn is the base class of all actors that can be possessed by players or AI.
 * They are the physical representations of players and creatures in a level.
 *
 * @see https://docs.unrealengine.com/latest/INT/Gameplay/Framework/Pawn/
 */
UCLASS(abstract, config=Game, BlueprintType, Blueprintable, hidecategories=(Navigation, "AI|Navigation"))
class ENGINE_API APawn : public AActor, public INavAgentInterface
{
	GENERATED_BODY()
public:

	/**
	 * Default UObject constructor.
	 */
	APawn(const FObjectInitializer& ObjectInitializer);

	/** Return our PawnMovementComponent, if we have one. By default, returns the first PawnMovementComponent found. Native classes that create their own movement component should override this method for more efficiency. */
	UFUNCTION(BlueprintCallable, meta=(Tooltip="Return our PawnMovementComponent, if we have one."), Category="Pawn")
	virtual UPawnMovementComponent* GetMovementComponent() const;

	/** Return PrimitiveComponent we are based on (standing on, attached to, and moving on). */
	virtual UPrimitiveComponent* GetMovementBase() const { return NULL; }

	/** If true, this Pawn's pitch will be updated to match the Controller's ControlRotation pitch, if controlled by a PlayerController. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Pawn)
	uint32 bUseControllerRotationPitch:1;

	/** If true, this Pawn's yaw will be updated to match the Controller's ControlRotation yaw, if controlled by a PlayerController. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Pawn)
	uint32 bUseControllerRotationYaw:1;

	/** If true, this Pawn's roll will be updated to match the Controller's ControlRotation roll, if controlled by a PlayerController. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Pawn)
	uint32 bUseControllerRotationRoll:1;

	/** If set to false (default) given pawn instance will never affect navigation generation. 
	 *	Setting it to true will result in using regular AActor's navigation relevancy 
	 *	calculation to check if this pawn instance should affect navigation generation
	 *	Use SetCanAffectNavigationGeneration to change this value at runtime.
	 *	Note that modifying this value at runtime will result in any navigation change only if runtime navigation generation is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn)
	uint32 bCanAffectNavigationGeneration : 1;

private:
	/* Whether this Pawn's input handling is enabled.  Pawn must still be possessed to get input even if this is true */
	uint32 bInputEnabled:1;

public:

	/** Base eye height above collision center. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Camera)
	float BaseEyeHeight;

	/* Specifies which player controller, if any, should automatically possess the pawn when the level starts or the pawn is spawned */
	UPROPERTY(EditAnywhere, Category=Pawn)
	TEnumAsByte<EAutoReceiveInput::Type> AutoPossess;

public:

	/**
	 * Return our PawnNoiseEmitterComponent, if any. Default implementation returns the first PawnNoiseEmitterComponent found in the components array.
	 * If one isn't found, then it tries to find one on the Pawn's current Controller.
	 */
	virtual UPawnNoiseEmitterComponent* GetPawnNoiseEmitterComponent() const;

	/**
	 * Inform AIControllers that you've made a noise they might hear (they are sent a HearNoise message if they have bHearNoises==true)
	 * The instigator of this sound is the pawn which is used to call MakeNoise.
	 *
	 * @param Loudness - is the relative loudness of this noise (range 0.0 to 1.0).  Directly affects the hearing range specified by the AI's HearingThreshold.
	 * @param NoiseLocation - Position of noise source.  If zero vector, use the actor's location.
	 * @param bUseNoiseMakerLocation - If true, use the location of the NoiseMaker rather than NoiseLocation.  If false, use NoiseLocation.
	 * @param NoiseMaker - Which actor is the source of the noise.  Not to be confused with the Noise Instigator, which is responsible for the noise (and is the pawn on which this function is called).  If not specified, the pawn instigating the noise will be used as the NoiseMaker
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=AI)
	void PawnMakeNoise(float Loudness, FVector NoiseLocation, bool bUseNoiseMakerLocation = true, AActor* NoiseMaker = NULL);

	/** default class to use when pawn is controlled by AI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=AI)
	TSubclassOf<AController>  AIControllerClass;

	/** If Pawn is possessed by a player, points to his playerstate.  Needed for network play as controllers are not replicated to clients. */
	UPROPERTY(replicatedUsing=OnRep_PlayerState, BlueprintReadOnly, Category="Pawn")
	APlayerState* PlayerState;

	/** Replicated so we can see where remote clients are looking. */
	UPROPERTY(replicated)
	uint8 RemoteViewPitch;

	/** Controller of the last Actor that caused us damage. */
	UPROPERTY(transient)
	AController* LastHitBy;

	/** Controller currently possessing this Actor */
	UPROPERTY(replicatedUsing=OnRep_Controller)
	AController* Controller;

	/** Max difference between pawn's Rotation.Yaw and GetDesiredRotation().Yaw for pawn to be considered as having reached its desired rotation */
	float AllowedYawError;

	/** Freeze pawn - stop sounds, animations, physics, weapon firing */
	virtual void TurnOff();

	/** Called when the Pawn is being restarted (usually by being possessed by a Controller). */
	virtual void Restart();

	/** Handle StartFire() passed from PlayerController */
	virtual void PawnStartFire(uint8 FireModeNum = 0);

	/**
	 * Set Pawn ViewPitch, so we can see where remote clients are looking.
	 * Maps 360.0 degrees into a byte
	 * @param	NewRemoteViewPitch	Pitch component to replicate to remote (non owned) clients.
	 */
	void SetRemoteViewPitch(float NewRemoteViewPitch);

	/** Called when our Controller no longer possesses us.	 */
	virtual void UnPossessed();

	/** Return Physics Volume for this Pawn **/
	virtual APhysicsVolume* GetPawnPhysicsVolume() const;

private:
	/** (DEPRECATED) @RETURN true if Pawn is currently walking (moving along the ground) */
	UFUNCTION(BlueprintCallable, Category="Character|Movement", meta=(DeprecatedFunction, DeprecationMessage="Query the movement component instead using IsMovingOnGround()."))
	virtual bool IsWalking() const;

	/** (DEPRECATED) @RETURN true if Pawn is currently falling */
	UFUNCTION(BlueprintCallable, Category="Character|Movement", meta=(DeprecatedFunction, DeprecationMessage="Query the movement component instead."))
	virtual bool IsFalling() const;

	/** (DEPRECATED) @RETURN true is Pawn is currently crouched */
	UFUNCTION(BlueprintCallable, Category="Character|Movement", meta=(DeprecatedFunction, DeprecationMessage="Query the movement component instead."))
	virtual bool IsCrouched() const;

public:
	/** Gets the owning actor of the Movement Base Component on which the pawn is standing. */
	UFUNCTION(BlueprintPure, Category="Pawn")
	static AActor* GetMovementBaseActor(const APawn* Pawn);

	virtual bool IsBasedOnActor(const AActor* Other) const override;

	virtual bool ReachedDesiredRotation();

	/** @return The half-height of the default Pawn, scaled by the component scale. By default returns the half-height of the RootComponent, regardless of whether it is registered or collidable. */
	virtual float GetDefaultHalfHeight() const;

	/** See if this actor is currently being controlled */
	UFUNCTION(BlueprintCallable, Category="Pawn")
	bool IsControlled() const;

	/** Returns controller for this actor. */
	UFUNCTION(BlueprintCallable, Category="Pawn")
	AController* GetController() const;

	/** Get the rotation of the Controller, often the 'view' rotation of this Pawn. */
	UFUNCTION(BlueprintCallable, Category="Pawn")
	FRotator GetControlRotation() const;

	/** Called when Controller is replicated */
	UFUNCTION()
	virtual void OnRep_Controller();

	/** PlayerState Replication Notification Callback */
	UFUNCTION()
	virtual void OnRep_PlayerState();

	/** used to prevent re-entry of OutsideWorldBounds event. */
	uint32 bProcessingOutsideWorldBounds:1;

	// Begin AActor Interface.
	virtual FVector GetVelocity() const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void Reset() override;
	virtual FString GetHumanReadableName() const override;
	virtual float GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, APlayerController* Viewer, UActorChannel* InChannel, float Time, bool bLowBandwidth) override;
	virtual bool ShouldTickIfViewportsOnly() const override;
	virtual bool IsNetRelevantFor(APlayerController* RealViewer, AActor* Viewer, const FVector& SrcLocation) override;
	virtual void PostNetReceiveLocationAndRotation() override;
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual void GetActorEyesViewPoint( FVector& Location, FRotator& Rotation ) const override;
	virtual void OutsideWorldBounds() override;
	virtual void Destroyed() override;
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	virtual UPlayer* GetNetOwningPlayer() override;
	virtual UNetConnection* GetNetConnection() override;
	virtual void PostLoad() override;
	virtual void PostRegisterAllComponents() override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void BecomeViewTarget(APlayerController* PC) override;
	virtual void EnableInput(APlayerController* PlayerController) override;
	virtual void DisableInput(APlayerController* PlayerController) override;

	/** Overridden to defer to the RootComponent's CanCharacterStepUpOn setting if it is explicitly Yes or No. If set to Owner, will return Super::CanBeBaseForCharacter(). */
	virtual bool CanBeBaseForCharacter(APawn* APawn) const override;
	// End AActor Interface

	/** Use SetCanAffectNavigationGeneration to change this value at runtime.
	 *	Note that calling this function at runtime will result in any navigation change only if runtime navigation generation is enabled. */
	UFUNCTION(BlueprintCallable, Category="Navigation")
	void SetCanAffectNavigationGeneration(bool bNewValue);

	/** update all components relevant for navigation generators to match bCanAffectNavigationGeneration flag */
	virtual void UpdateNavigationRelevance() {}

	// Begin INavAgentInterface Interface
	virtual const struct FNavAgentProperties* GetNavAgentProperties() const override;
	/** Basically retrieved pawn's location on navmesh */
	UFUNCTION(BlueprintCallable, Category="Pawn")
	virtual FVector GetNavAgentLocation() const override { return GetActorLocation() - FVector(0.f, 0.f, BaseEyeHeight); }
	virtual void GetMoveGoalReachTest(AActor* MovingActor, const FVector& MoveOffset, FVector& GoalOffset, float& GoalRadius, float& GoalHalfHeight) const override;
	// End INavAgentInterface Interface

	/** Allows agent to postpone any path updates (e.g. locked by gameplay) */
	virtual bool ShouldPostponePathUpdates() const { return false; }

	/** updates MovementComponent's parameters used by navigation system */
	void UpdateNavAgent();

	/** @return true if we are in a state to take damage (checked at the start of TakeDamage.
	*   Subclasses may check this as well if they override TakeDamage and don't want to potentially trigger TakeDamage actions by checking if it returns zero in the super class. */
	virtual bool ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

#if WITH_EDITOR
	virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown);
#endif

	/** @return vector direction of gravity */
	FVector GetGravityDirection();

	/** Make sure pawn properties are back to default. */
	virtual void SetPlayerDefaults();

	/** Set BaseEyeHeight based on current state. */
	virtual void RecalculateBaseEyeHeight();

	bool InputEnabled() const { return bInputEnabled; }

	/** 
	 * Called when this Pawn is possessed. Only called on the server (or in standalone).
	 *	@param C The controller possessing this pawn
	 */
	virtual void PossessedBy(AController* NewController);

	/** Event called when the Pawn is possessed by a Controller (normally only occurs on the server/standalone). */
	UFUNCTION(BlueprintImplementableEvent, meta=(FriendlyName = "Possessed"))
	void ReceivePossessed(AController* NewController);

	/** Event called when the Pawn is no longer possessed by a Controller. */
	UFUNCTION(BlueprintImplementableEvent, meta=(FriendlyName = "Unpossessed"))
	void ReceiveUnpossessed(AController* OldController);

	/** @return true if controlled by a local (not network) Controller.	 */
	UFUNCTION(BlueprintCallable, Category="Pawn")
	virtual bool IsLocallyControlled() const;

	/**
	 * Get the view rotation of the Pawn (direction they are looking, normally Controller->ControlRotation).
	 * @return The view rotation of the Pawn.
	 */
	virtual FRotator GetViewRotation() const;

	/** @return	Pawn's eye location */
	virtual FVector GetPawnViewLocation() const;

	/**
	 * Return the aim rotation for the Pawn.
	 * If we have a controller, by default we aim at the player's 'eyes' direction
	 * that is by default the Pawn rotation for AI, and camera (crosshair) rotation for human players.
	 */
	UFUNCTION(BlueprintCallable, Category="Pawn")
	virtual FRotator GetBaseAimRotation() const;

	/** return true if player is viewing this Pawn in FreeCam */
	virtual bool InFreeCam() const;

	/** Tell client that the Pawn is begin restarted. Calls Restart(). */
	virtual void PawnClientRestart();

	/** Replicated function to set the pawn rotation, allowing the server to force. */
	virtual void ClientSetRotation( FRotator NewRotation );

	/** Updates Pawn's rotation to the given rotation, assumed to be the Controller's ControlRotation. Respects the bUseControllerRotation* settings. */
	virtual void FaceRotation(FRotator NewControlRotation, float DeltaTime = 0.f);

	/** Call this function to detach safely pawn from its controller, knowing that we will be destroyed soon.	 */
	UFUNCTION(BlueprintCallable, Category="Pawn", meta=(Keywords = "Delete"))
	virtual void DetachFromControllerPendingDestroy();

	/** Spawn default controller for this Pawn, and get possessed by it. */
	UFUNCTION(BlueprintCallable, Category="Pawn")
	virtual void SpawnDefaultController();

protected:
	/** Get the controller instigating the damage. If the damage is caused by the world and the supplied controller is NULL or is this pawn's controller, uses LastHitBy as the instigator. */
	virtual AController* GetDamageInstigator(AController* InstigatedBy, const UDamageType& DamageType) const;

	/** Creates an InputComponent that can be used for custom input bindings. Called upon possession by a PlayerController. Return null if you don't want one. */
	virtual UInputComponent* CreatePlayerInputComponent();

	/** Destroys the player input component and removes any references to it. */
	virtual void DestroyPlayerInputComponent();

	/** Allows a Pawn to set up custom input bindings. Called upon possession by a PlayerController, using the InputComponent created by CreatePlayerInputComponent(). */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) { /* No bindings by default.*/ }

public:
	/**
	 * Add movement input along the given world direction vector (usually normalized) scaled by 'ScaleValue'. If ScaleValue < 0, movement will be in the opposite direction.
	 * Base Pawn classes won't automatically apply movement, it's up to the user to do so in a Tick event. Subclasses such as Character and DefaultPawn automatically handle this input and move.
	 *
	 * @param WorldDirection	Direction in world space to apply input
	 * @param ScaleValue		Scale to apply to input. This can be used for analog input, ie a value of 0.5 applies half the normal value, while -1.0 would reverse the direction.
	 * @param bForce			If true always add the input, ignoring the result of IsMoveInputIgnored().
	 * @see GetPendingMovementInputVector(), GetLastMovementInputVector(), ConsumeMovementInputVector()
	 */
	UFUNCTION(BlueprintCallable, Category="Pawn|Input", meta=(Keywords="AddInput"))
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false);

	/**
	 * Return the pending input vector in world space. This is the most up-to-date value of the input vector, pending ConsumeMovementInputVector() which clears it,
	 * Usually only a PawnMovementComponent will want to read this value, or the Pawn itself if it is responsible for movement.
	 *
	 * @return The pending input vector in world space.
	 * @see AddMovementInput(), GetLastMovementInputVector(), ConsumeMovementInputVector()
	 */
	UFUNCTION(BlueprintCallable, Category="Pawn|Input", meta=(Keywords="GetMovementInput GetInput"))
	FVector GetPendingMovementInputVector() const;

	/**
	 * Return the last input vector in world space that was processed by ConsumeMovementInputVector(), which is usually done by the Pawn or PawnMovementComponent.
	 * Any user that needs to know about the input that last affected movement should use this function.
	 * For example an animation update would want to use this, since by default the order of updates in a frame is:
	 * PlayerController (device input) -> MovementComponent -> Pawn -> Mesh (animations)
	 *
	 * @return The last input vector in world space that was processed by ConsumeMovementInputVector().
	 * @see AddMovementInput(), GetPendingMovementInputVector(), ConsumeMovementInputVector()
	 */
	UFUNCTION(BlueprintCallable, Category="Pawn|Input", meta=(Keywords="GetMovementInput GetInput"))
	FVector GetLastMovementInputVector() const;

	/**
	 * Returns the pending input vector and resets it to zero.
	 * This should be used during a movement update (by the Pawn or PawnMovementComponent) to prevent accumulation of control input between frames.
	 * Copies the pending input vector to the saved input vector (GetLastMovementInputVector()).
	 * @return The pending input vector.
	 */
	UFUNCTION(BlueprintCallable, Category="Pawn|Input", meta=(Keywords="ConsumeInput"))
	virtual FVector ConsumeMovementInputVector();

	/** Add input (affecting Pitch) to the Controller's ControlRotation, if it is a local PlayerController. */
	UFUNCTION(BlueprintCallable, Category="Pawn|Input", meta=(Keywords="up down"))
	virtual void AddControllerPitchInput(float Val);

	/** Add input (affecting Yaw) to the Controller's ControlRotation, if it is a local PlayerController. */
	UFUNCTION(BlueprintCallable, Category="Pawn|Input", meta=(Keywords="left right turn"))
	virtual void AddControllerYawInput(float Val);

	/** Add input (affecting Roll) to the Controller's ControlRotation, if it is a local PlayerController. */
	UFUNCTION(BlueprintCallable, Category="Pawn|Input")
	virtual void AddControllerRollInput(float Val);

	/** Helper to see if move input is ignored. If our controller is a PlayerController, checks Controller->IsMoveInputIgnored(). */
	UFUNCTION(BlueprintCallable, Category="Pawn|Input")
	virtual bool IsMoveInputIgnored() const;

protected:

	/**
	 * Accumulated control input vector, stored in world space. This is the pending input, which is cleared (zeroed) once consumed.
	 * @see GetPendingMovementInputVector(), AddMovementInput()
	 */
	UPROPERTY(Transient)
	FVector ControlInputVector;

	/**
	 * The last control input vector that was processed by ConsumeMovementInputVector().
	 * @see GetLastMovementInputVector()
	 */
	UPROPERTY(Transient)
	FVector LastControlInputVector;

public:
	/** Internal function meant for use only within Pawn or by a PawnMovementComponent. Adds movement input if not ignored, or if forced. */
	void Internal_AddMovementInput(FVector WorldAccel, bool bForce = false);

	/** Internal function meant for use only within Pawn or by a PawnMovementComponent. Returns the value of ControlInputVector. */
	inline FVector Internal_GetPendingMovementInputVector() const { return ControlInputVector; }

	/** Internal function meant for use only within Pawn or by a PawnMovementComponent. Returns the value of LastControlInputVector. */
	inline FVector Internal_GetLastMovementInputVector() const { return LastControlInputVector; }

	/** Internal function meant for use only within Pawn or by a PawnMovementComponent. LastControlInputVector is updated with initial value of ControlInputVector. Returns ControlInputVector and resets it to zero. */
	FVector Internal_ConsumeMovementInputVector();

public:

	/** Add an Actor to ignore by Pawn's movement collision */
	void MoveIgnoreActorAdd(AActor* ActorToIgnore);

	/** Remove an Actor to ignore by Pawn's movement collision */
	void MoveIgnoreActorRemove(AActor* ActorToIgnore);

public:

	// DEPRECATED FUNCTIONS

	/** (Deprecated) Launch Character with LaunchVelocity  */
	UFUNCTION(BlueprintCallable, Category="Pawn", meta=(DeprecatedFunction, DeprecationMessage="Use Character.LaunchCharacter instead"))
	void LaunchPawn(FVector LaunchVelocity, bool bXYOverride, bool bZOverride);

	/** Internal function meant for use only within Pawn or by a PawnMovementComponent. */
	DEPRECATED(4.5, "Internal_GetMovementInputVector is deprecated, use Internal_GetPendingMovementInputVector() or Internal_GetLastMovementInputVector() instead.")
	inline FVector Internal_GetMovementInputVector() const { return Internal_GetPendingMovementInputVector(); }

	/** (Deprecated) Return the input vector in world space. */
	DEPRECATED(4.5, "GetMovementInputVector() has been deprecated, use either GetPendingMovementInputVector() or GetLastMovementInputVector().")
	FVector GetMovementInputVector() const;

	/** (Deprecated) Return the input vector in world space. */
	UFUNCTION(BlueprintCallable, Category="Pawn|Input", meta=(DeprecatedFunction, FriendlyName="GetMovementInputVector", DeprecationMessage="GetMovementInputVector has been deprecated, use either GetPendingMovementInputVector or GetLastMovementInputVector"))
	FVector K2_GetMovementInputVector() const;	
};

