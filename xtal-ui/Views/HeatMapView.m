//
//  HeatMapView.m
//  xtal-ui
//
//  Created by Simon Gornall on 1/3/23.
//

#import "HeatMapView.h"

@implementation HeatMapView

- (void)drawRect:(NSRect)dirtyRect
	{
    [super drawRect:dirtyRect];
    
    [[NSColor blueColor] setFill];
    NSRectFill(dirtyRect);
	}

@end
