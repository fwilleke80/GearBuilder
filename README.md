# GearBuilder

GearBuilder is a small but nevertheless helpful plugin for building almost any kind of gear.

To make it simple, GearBuilder is an expression for constraining the rotation and position of objects to emulate the behaviour of cogwheels, wheels on a a shared axis and wheels connected by belts. The created gears can either be driven by a GearBuilder motor or simply by keyframing the rotation of the initial wheel.

This plugin demonstrates the following C4D API aspects:
* Expression tag plugins, derived from `class TagData`
* Custom tag icons using `MSG_GETCUSTOMICON`
* Dynamic GUI layout with `NodeData::GetDDescription()`
