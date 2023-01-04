//
//  CpuStateView.m
//  xtal-ui
//
//  Created by Simon Gornall on 1/3/23.
//

#import "CpuStateView.h"

@implementation CpuStateView

- (void)drawRect:(NSRect)dirtyRect
	{
    [super drawRect:dirtyRect];
    
    [[NSColor greenColor] setFill];
    NSRectFill(dirtyRect);
	}

@end
