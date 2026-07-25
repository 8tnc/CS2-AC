# CS2AC

CS2AC is a server-side anti-cheat plugin for Counter-Strike 2 and Metamod:Source. It watches player input and gameplay for clearly suspicious behavior, announces detections in chat and on screen, and can run administrator-defined punishment commands.

The plugin includes 18 detectors: Aimbot, Aimlock, Antiaim, Autostrafe, Bhop, DLL Injection, Desubticking, Doubletap, Hyperscroll, Inhuman Accuracy, Invalid CVar, Invalid Input, Irregular Behavior, Namechanger, Nulls, Silentaim, Subtick Spam, and Triggerbot.

## Requirements

- A Windows x64 or Linux x64 Counter-Strike 2 dedicated server
- [Metamod:Source](https://www.sourcemm.net/) 2.x
- An administrator plugin if you want CS2AC to punish players through console commands

The default punishment commands use [CS2-SimpleAdmin](https://github.com/daffyyyy/CS2-SimpleAdmin). You can replace them with commands from any administrator plugin.

## Installation

1. Download the Windows or Linux ZIP from the latest GitHub release.
2. Extract it into the Counter-Strike 2 server root. The archive starts with the `game` folder.
3. Edit `game/csgo/cfg/cs2ac.cfg`.
4. Start or restart the server.
5. Run `meta list` and `cs2ac_status` in the server console.

CS2AC loads `cs2ac.cfg` automatically. All detectors are enabled by default.

## Configuration

The included configuration file explains every setting. The main options are:

| Setting | Purpose |
| --- | --- |
| `cs2ac_enabled` | Enables or disables all detection. |
| `cs2ac_whitelist` | Comma-separated SteamID64 values that may be detected and announced but never punished. |
| `cs2ac_*_enabled` | Enables or disables one detector. |
| `cs2ac_chat_announcements` | Shows public detection messages in chat. |
| `cs2ac_center_announcements` | Shows the center-screen detection alert. |
| `cs2ac_punishment_command` | Runs for permanent-ban detections. |
| `cs2ac_kick_command` | Runs for detections configured as a kick. |
| `cs2ac_webhook_url` | Sends detection reports to a Discord webhook. |
| `cs2ac_webhook_role_id` | Optionally mentions a Discord role in reports. |
| `cs2ac_webhook_server_address` | Overrides the automatically detected public server address. |
| `cs2ac_webhook_logo_url` | Sets the Discord branding and fallback image when a Steam avatar is unavailable. |
| `cs2ac_allow_sv_cheats_testing` | Allows local detector testing while `sv_cheats 1` is active. Keep it off publicly. |

Punishment commands support `{steamid64}`, `{userid}`, and `{detection}`. Leave a command empty to disable that punishment type.

Example whitelist:

```cfg
cs2ac_whitelist "76561198000000001,76561198000000002"
```

## Administrator commands

| Command | Purpose |
| --- | --- |
| `cs2ac_status` | Shows plugin, player, detector, announcement, and punishment status. |
| `cs2ac_help` | Lists the administrator commands. |
| `cs2ac_reload` | Reloads and validates `cs2ac.cfg`. |
| `cs2ac_check_config` | Checks the current settings without changing them. |
| `cs2ac_test_announcement` | Shows a harmless chat and center-screen test. |
| `cs2ac_webhook_test` | Sends a harmless Discord test report. |

## Discord reports

Create a Discord webhook, put its URL in `cs2ac_webhook_url`, then run `cs2ac_reload` and `cs2ac_webhook_test`. Keep the webhook URL private. A failed webhook is disabled until the configuration is reloaded, so a broken endpoint cannot repeatedly delay the server.

## Building

Clone the repository with its pinned submodules:

```sh
git clone --recursive https://github.com/karola3vax/CS2AC.git
cd CS2AC
```

Windows requires Python 3.8 or newer and Visual Studio 2022 with the C++ workload:

```powershell
./build-windows.ps1
```

Linux requires Python 3.8 or newer and Docker:

```sh
./build-linux.sh
```

Both scripts download the pinned AMBuild revision. Linux compilation runs inside the pinned Steam Runtime 3 SDK image. Finished, directly installable files are placed under the build folder's `package/game` directory.

Pushes and pull requests build both platforms in GitHub Actions. Pushing a signed or annotated `v*` tag creates a GitHub release containing the Windows and Linux ZIPs.

## License

CS2AC is licensed under the [GNU Affero General Public License v3.0](LICENSE). Its pinned SDK, build, and vendored dependencies keep their own licenses; see [Third-party notices](THIRD_PARTY_NOTICES.md). Binary archives include the applicable license texts.
