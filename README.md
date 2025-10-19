# Ninja GASP

This project integrates [Polygon Hive][1]'s GASP-ALS project - a GASP tempalte with ALS Overlay Layering System - with the suite of plugins from [Ninja Bear Studio][2].
It's designed as a self-contained template, but internal plugins are isolataed and should be transferrable to any project.

## Main Features
- All features from the original GASP-ASL project, includin a deep overlaying system, ragdolls and animation stances.
- Integrated with Ninja Bear Studio plugins, including additional integration code, as needed.
- Features are fully compartimentalized into plugis so you can easily transfer them to your own project.

## Pre-requisites
First and foremost, a good understanding of **GASP** and the **ALS layering system**, is important so you can extend the system. You'll find multiple videos on this topic
in [Polygon Hive's YouTube channel][1]. This sample also includes all plugins from Ninja Bear Studio, so you'll only be able to execute the project if you have all necessary
licenses, which are available on [Fab][3].

The core NinjaGASP plugin includes **C++ sources** so your system must be configured to compile the project. You don't need to do so from an IDE and you don't need to deal
with C++ yourself, but you will have to configure your machine following [Unreal Engine's installation steps for C++][4].

## Plugin Integration
This project does not include commercial plugins and you can install them using one of the following options:

- **Epic Games Launcher**: You can install all Ninja Bear Studio plugins to your engine, using the Launcher.
- **GitHub Sources**: You can clone plugins from GitHub into the project. These sources are ignored and won't be pushed to repository. This project includes scripts to initialize and update git dependencies.

[1]: https://www.youtube.com/watch?v=RDWNfIqvWBk&list=PLs9e0eJQMI2aaulgKJzC8feN1UEwDkEnq
[2]: https://tech.ninjabear.studio/
[3]: https://www.fab.com/sellers/Ninja%20Bear%20Studio
[4]: https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine
