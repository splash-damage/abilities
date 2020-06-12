# Ability System

Splash Damage's **Ability System** (**SAS**) is an **Unreal Engine 4** plugin that provides lightweight, intuitive and efficient abilities.

**SAS** is designed to provide developers flexibility and better quality of life while building content for their games. It **empowers developers** to create games with complex ability designs with the minimum amount of work.

The uses of SAS can be broad. It is **not** designed to suit a particular genre. However it will, at least conceptually, fit better in genres like FPS, RPGs, Combat, Stealth, RTS, etc.

## Introduction

If this is the first time you read about SAS, you will have many questions to be answered, but lets first start with the two most important concepts:

### Ability

An Ability is **an action** triggered by a player or AI that contains logic and state. It is mutable and instanced for each owner. Abilities can be replicated and predicted in multiplayer projects.

### Buff

A Buff represents **an static state or condition** applied to a player or AI. Buffs are never modified in runtime and their effects are deterministic. They are not instanced unlike abilities.

## What is not SAS?

SAS has many good features but it has a certain scope and wont implement any of the following out of the box:

- UI of any type
- Prebuilt game content
- Specific cosmetic features (VFX, Animation, Audio...)
- Gameplay attributes

The API is very extensive and flexible. All this areas will of course connect effortlessly to the system. For Attributes, there are multiple existing solutions. One example is [Attributes Extension](https://piperift.com/AttributesExtension/).

## Documentation

**If you have never used SAS before**, consider reading through **[First Steps](quick-start/first-steps.md)**. This guide will help you understand the *HOWs* and the *WHYs* of this system, simplifying more complicated concepts later.

Those familiarized with the plugin will still find more information in **Quick Steps** about topics like content creation, networking, cosmetics, debugging and more.

Finally, those programmers with interest in learning about the **guts of SAS** can check **Resources** for in-depth views of the plugin. This includes architecture, design & development principles and low-level ability features.
