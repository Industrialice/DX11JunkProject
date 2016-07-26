#ifndef __SPLASH_SCREEN_HPP__
#define __SPLASH_SCREEN_HPP__

namespace SplashScreen
{
    void Step();
	bln IsActiveGet();
	void IsActiveSet( bln is_active );
    void Create( bln is_active );
}

#endif __SPLASH_SCREEN_HPP__