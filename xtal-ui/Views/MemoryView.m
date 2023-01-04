//
//  MemoryView.m
//  xtal-ui
//
//  Created by Simon Gornall on 1/3/23.
//

#import "MemoryView.h"

@implementation MemoryView

- (void)drawRect:(NSRect)dirtyRect
	{
    [super drawRect:dirtyRect];
    
    [[NSColor redColor] setFill];
    NSRectFill(dirtyRect);
	}

@end
