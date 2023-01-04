//
//  AssemblyView.m
//  xtal-ui
//
//  Created by Simon Gornall on 1/3/23.
//

#import "AssemblyView.h"

@implementation AssemblyView

- (void)drawRect:(NSRect)dirtyRect
	{
    [super drawRect:dirtyRect];
    
    [[NSColor purpleColor] setFill];
    NSRectFill(dirtyRect);
	}

@end
