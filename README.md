# SvenBXT

> **Compatibility note:** This fork contains crash-safety changes, not a verified reverse-engineering update of the 5.26 engine signatures. Test on a clean Sven Co-op 5.26 install before relying on it.

SvenBXT is a remake of [BunnymodXT](https://github.com/YaLTeR/BunnymodXT), originally developed for Sven Co-op 5.25.
<br>You can legally use this tool for [Sven Co-op runs](https://www.speedrun.com/sven_coop).</br>

Download the prebuilt Windows x86 DLL: [Nightly Release](https://github.com/Debiloid3000/SvenBXT/releases/tag/nightly) (updated automatically after every successful push to `new`) or [all Releases](https://github.com/Debiloid3000/SvenBXT/releases).
CI builds (before a release): [GitHub Actions](https://github.com/Debiloid3000/SvenBXT/actions/workflows/CI.yml).

To publish `SvenBXT.dll` automatically, push a `v*` tag or publish a GitHub Release. The release workflow builds a 32-bit Windows DLL and attaches it directly to that release.

## Supported versions
- 5.11
- 5.22
- 5.23
- 5.25
- 5.26-rc1 ([NOT SUPPORTED ON LINUX](https://github.com/ScriptedSnark/SvenBXT/issues/11))
- 5.26 ([NOT SUPPORTED ON LINUX](https://github.com/ScriptedSnark/SvenBXT/issues/11))

Sven Co-op 4.x or older are not planned to be supported.

## Installation / Usage

### Windows
SvenBXT can be installed as a [MetaHook](https://github.com/hzqst/MetaHookSv)/[SvenMod](https://github.com/sw1ft747/svenmod)/[StarLoader](https://github.com/autisoid/StarLoader) plugin.
Also can be injected by any injector (even deprecated one from `rebased` branch).

#### MetaHook
1. Move `SvenBXT.dll` to `svencoop/metahook/plugins`
2. Write `SvenBXT.dll` in `svencoop/metahook/configs/plugins.lst`

#### SvenMod
1. Move `SvenBXT.dll` to `svenmod/plugins`
2. Open `svenmod/plugins.txt` and write `"SvenBXT"			"1"` inside plugins tab

```
// SvenMod
// List of plugins to load

"Plugins"
{
	"SvenBXT"			"1"
	
	"Settings"
	{
		"LoadDelay"				"0.5" // in seconds
	}
}
```

You can find additional info about that [here](https://github.com/sw1ft747/svenmod#adding-plugins).


#### StarLoader
1. Move `SvenBXT.dll` to `starloader` directory
2. Rename `SvenBXT.dll` to `SvenBXT.asi`

### Linux
You can use the `runsven.sh` script. Correct the paths at the top of the script if necessary.

## Credits
**Thanks to [YaLTeR](https://github.com/YaLTeR) for [BunnymodXT](https://github.com/YaLTeR/BunnymodXT)!**

Thanks to [autisoid](https://github.com/autisoid) for SvenMod/MetaHook emulators.

## Crash diagnosis (Windows)

If Sven Co-op shows a generic fatal error after loading SvenBXT, first reproduce without SvenBXT and without other MetaHook plugins. Then try SvenBXT as the only third-party plugin and check `svenbxt.log` in the game working directory. Unknown engine signatures are now skipped instead of being installed as invalid hooks.

The existing 5.26/5.26-rc1 pattern data has **not** been revalidated against a current engine DLL. If the game still crashes, attach the `svenbxt.log`, Sven Co-op build number, MetaHookSv version, and a Windows crash dump/backtrace when filing an issue. A screenshot of the generic error alone cannot locate the crashing function.

## Independent repository copy (not an Actions workflow)

Run [scripts/create_standalone_repo.ps1](scripts/create_standalone_repo.ps1) in PowerShell after installing Git and GitHub CLI and running `gh auth login`. It clones the current `new` branch with its history/submodules, removes GitHub Actions workflows from that new independent copy by default, and uses GitHub CLI to create `Debiloid3000/SvenBXT-Standalone`. You can change the destination name with `-DestinationRepo`.

Creating a new GitHub repository is an authenticated operation: the script must be run using an account authorized to create repositories.
