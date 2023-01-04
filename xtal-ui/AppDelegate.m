//
//  AppDelegate.m
//  xtal-ui
//
//  Created by Simon Gornall on 1/3/23.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@property (strong) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
	{
	// Insert code here to initialize your application
	}


- (void)applicationWillTerminate:(NSNotification *)aNotification
	{
	// Insert code here to tear down your application
	}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app
	{
	return YES;
	}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
	{
	return YES;
	}

@end
