//
//  VcrControlsView.m
//  xtal-ui
//
//  Created by Simon Gornall on 1/3/23.
//

#import "VcrControlsView.h"

@implementation VcrControlsView

- (void)drawRect:(NSRect)dirtyRect
	{
    [super drawRect:dirtyRect];
    
    [[NSColor brownColor] setFill];
    NSRectFill(dirtyRect);
	}

@end
