//
//  PointsView.m
//  xtal-ui
//
//  Created by Simon Gornall on 1/3/23.
//

#import "PointsView.h"

@implementation PointsView

- (void)drawRect:(NSRect)dirtyRect
	{
    [super drawRect:dirtyRect];
    
    [[NSColor grayColor] setFill];
    NSRectFill(dirtyRect);
	}

@end
