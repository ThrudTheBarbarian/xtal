//
//  HeatMapControlsView.m
//  xtal-ui
//
//  Created by Simon Gornall on 1/3/23.
//

#import "HeatMapControlsView.h"

@implementation HeatMapControlsView

- (void)drawRect:(NSRect)dirtyRect
	{
    [super drawRect:dirtyRect];
    
    [[NSColor orangeColor] setFill];
    NSRectFill(dirtyRect);
	}

@end
