#pragma once

class Player;

class Interactable
{
public:

	virtual ~Interactable() = default;

	virtual void Interact(std::shared_ptr<Player> player) = 0;

	virtual bool CanInteract() const
	{
		return true;
	}

	virtual std::string GetInteractText() const
	{
		return "Interact";
	}
};