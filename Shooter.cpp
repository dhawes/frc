#include "Shooter.h"
#include "Sparky.h"

Shooter::Shooter():
	tension(1, 2), shooter(12), trigger(11), release(6), arm(1)
{
	armSet = false;
	setEncPos(0);
	zero();
	tension.Start();
}

void Shooter::ReleaseNotifier(void* p)
{
	printf("ReleaseNotifier: start\n");
	Sparky *s = (Sparky *)p;
	Loader *l = s->GetLoader();
	Shooter *sh = s->GetShooter();
	{
		Synchronized sync(releaseSem);
		while(sh->getTrigger() && s->IsEnabled())
		{
			sh->setRelease(Relay::kReverse);
			Wait(0.005);
		}
		Wait(0.1);
		sh->setRelease(Relay::kOff);
		Wait(0.3);
		releaseSet = false;
		l->setIntakeOff(true);
		armSet = true;
		s->ArmToPositionFull(0);
		while(s->GetTension() > ARM_ZERO_THRESH && s->IsEnabled())
		{
			Wait(0.1);
		}
		while(l->getTop() && s->IsEnabled())
		{
			l->load();
			Wait(0.005);
		}
		Wait(1.0);
		l->stop();
		s->ArmToPosition(125);
		l->setIntakeOff(false);
		armSet = false;
		printf("ReleaseNotifier: done\n");
	}
}

void Shooter::setArmSet(bool b)
{
	armSet = b;
}

bool Shooter::isArmSet()
{
	return armSet;
}

int Shooter::getEncPos()
{
	return encPos;
}

void Shooter::setEncPos(int i)
{
	encPos = i;
}

double Shooter::getArmSpeed()
{
	return armSpeed;
}

void Shooter::setArmSpeed(double d)
{
	armSpeed = d;
}

void Shooter::setReleaseSet(bool b)
{
	releaseSet = b;
}

bool Shooter::isReleaseSet()
{
	return releaseSet;
}

int Shooter::getTension()
{
	return tension.Get();
}

void Shooter::zero()
{
	tension.Reset();
}

void Shooter::load(double s)
{
	arm.Set(s);
}

void Shooter::setRelease(Relay::Value v)
{
	release.Set(v);
}

int Shooter::getTrigger()
{
	return trigger.Get();
}

void Shooter::ArmToPositionNotifier(void* p)
{
	Sparky *s = (Sparky *)p;
	Shooter *sh = s->GetShooter();
	Loader *l = s->GetLoader();
	{
		Synchronized sync(armSem);
		if(s->GetTension() < sh->getEncPos() && l->getShooter())
		{
			while(s->GetTension() < sh->getEncPos())
			{
				sh->load(-sh->getArmSpeed());
				Wait(0.005);
			}
		}
		else if(s->GetTension() > sh->getEncPos())
		{
			while(s->GetTension() > sh->getEncPos())
			{
				sh->load(sh->getArmSpeed());
				Wait(0.005);
			}
		}
		sh->load(TENSION_BRAKE);
		sh->setArmSet(false);
	}
}
