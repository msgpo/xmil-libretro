/*
 *	@file	main.m
 */

#import "compiler.h"
#include "../xmil.h"
#include "../dosio.h"

int SDL_main(int argc, char *argv[])
{
	NSArray *paths;
	NSString *DocumentsDirPath;
	char path[MAX_PATH];

	paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	DocumentsDirPath = [paths objectAtIndex:0];

	NSString *current = [DocumentsDirPath stringByAppendingString:@"/"];
	file_setcd([current UTF8String]);

	return xmil_main(argc, argv);
}
