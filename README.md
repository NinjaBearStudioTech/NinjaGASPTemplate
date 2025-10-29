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

- **Epic Games Launcher**: You can install all Ninja Bear Studio plugins to your engine, using the Launcher. (_soon, see note_)
- **GitHub Sources**: You can clone plugins from GitHub into the project. These sources are ignored and won't be pushed to repository. This project includes scripts to initialize and update git dependencies.

> **This is WIP**
> this project is under development and it's only guaranteed to work with GIT versions of the plugins. Once the template stabilizes, it should
> always work with both Fab and GitHub versions. For the time being, make sure to use GH. Make sure to check the scripts listed below.

## GitHub Plugin Scripts
You will find in the `/scripts/` folder two relevant commands that can help synchronizing with GitHub sources. Keep in mind that, in order to use these
scripts, you **must have access to GitHub repositories**. You can request access by **verifying your licenses in the [support server](5)**.

- `initialize_dependencies.bat`: Initializes all dependencies in the right location. Make sure to run them for the first time when you clone.
- `update_plugins.bat`: Updates all plugins from their repositories.

## License and Support
This template is provided as-is, and while I offer best-effort support, there are no guarantees of direct assistance.
For community help and discussion, please visit our [Discord server][5].

[1]: https://www.youtube.com/watch?v=RDWNfIqvWBk&list=PLs9e0eJQMI2aaulgKJzC8feN1UEwDkEnq
[2]: https://tech.ninjabear.studio/
[3]: https://www.fab.com/sellers/Ninja%20Bear%20Studio
[4]: https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine
[5]: https://discord.gg/mkB5cQmDws
