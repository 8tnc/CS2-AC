<div align="center">

# CS2AC

### Server-side anti-cheat for Counter-Strike 2 community servers

[![Build](https://img.shields.io/github/actions/workflow/status/karola3vax/CS2AC/build.yml?branch=main&style=for-the-badge&label=build)](https://github.com/karola3vax/CS2AC/actions/workflows/build.yml) [![Version](https://img.shields.io/badge/version-1.0.0-blue?style=for-the-badge)](https://github.com/karola3vax/CS2AC) [![Detectors](https://img.shields.io/badge/detectors-17-red?style=for-the-badge)](#what-it-detects) [![License](https://img.shields.io/badge/license-AGPL--3.0-green?style=for-the-badge)](LICENSE)

<!-- Add docs/detection.gif here when the original CS2AC chat and center-screen demonstration is ready.
<img src="docs/detection.gif" width="800" alt="CS2AC announcing a confirmed detection in chat and at the center of the screen">
-->

</div>

CS2AC watches the commands and gameplay that reach your Counter-Strike 2 server for behavior a normal client should not produce. When it confirms a detection, it can warn everyone in chat and at the center of the screen, send a detailed Discord report, and pass the punishment to your existing administrator plugin.

- **Everything runs on your server:** players install nothing.
- **Every detector is optional:** the included configuration keeps control in the server owner's hands.
- **Punishments are yours:** CS2AC runs the command you configure instead of tying you to one admin system.
- **Whitelisted players stay visible:** their detections are still announced and reported, but no punishment command is sent.

## What it detects

All 17 detectors are enabled by default.

| Aim and combat | Movement | Client and input |
| --- | --- | --- |
| **Aimbot** — damaging visible aim snaps | **Autostrafe** — automated air strafing | **DLL Injection** — suspicious client event subscriptions |
| **Aimlock** — unnaturally precise target tracking | **Bhop** — automated bunny hopping | **Desubticking** — commands with normal subtick timing removed |
| **Silentaim** — bullets that disagree with visible aim | **Hyperscroll** — automated jump-input frequency | **Doubletap** — impossible rapid fire |
| **Inhuman Accuracy** — sustained near-perfect results | **Nulls** — automated opposite-direction input timing | **Invalid CVar** — unsafe or manipulated client settings |
| **Irregular Behavior** — repeated difficult airborne or unscoped results |  | **Invalid Input** — malformed player commands |
|  |  | **Antiaim** — impossible or manipulated view angles |
|  |  | **Namechanger** — repeated rapid name changes |
|  |  | **Subtick Spam** — an impossible number of subtick inputs |

## What a detection looks like

```text
[CS2AC] detected AIMBOT on Player and punished.
```

The public announcement uses a red CS2AC prefix, a lime detection name, a grey player name, and white sentence text. The center alert repeats briefly so it cannot disappear between client UI updates.

<!-- Add docs/announcements.gif here when the original CS2AC announcement demonstration is ready.
<img src="docs/announcements.gif" width="800" alt="CS2AC showing the same detection in chat and through its center-screen alert">
-->

Discord reports include the player, SteamID64, detection, collected evidence, punishment result, server, map, address, and Steam avatar when one is available.

<!-- Add docs/webhook.gif here when the original CS2AC Discord report demonstration is ready.
<img src="docs/webhook.gif" width="800" alt="CS2AC sending a detailed detection report to Discord">
-->

## FAQ

<details>
<summary><strong>Does anything run on the player's computer?</strong></summary>

No. CS2AC is a Metamod:Source plugin that runs inside the dedicated server. Players join normally and download no anti-cheat client.

</details>

<details>
<summary><strong>Does it work in Premier or Valve matchmaking?</strong></summary>

No. You need control of a community or dedicated server running Metamod:Source.

</details>

<details>
<summary><strong>Does CS2AC ban players by itself?</strong></summary>

CS2AC decides when to act, then sends the console command configured by the server owner. The defaults use CS2-SimpleAdmin, but any administrator plugin with a suitable command can be used.

</details>

<details>
<summary><strong>Which detections ban and which only kick?</strong></summary>

Desubticking, Nulls, and Subtick Spam use the configured kick command. The other detections use the configured permanent-ban command. Emptying either command disables that punishment type without hiding the detection.

</details>

<details>
<summary><strong>What happens to whitelisted players?</strong></summary>

Their detections still appear in chat, on screen, and in Discord. CS2AC clearly says that the player is whitelisted and does not send a punishment command.

</details>

<details>
<summary><strong>Can CS2AC catch every cheat?</strong></summary>

No server-side anti-cheat can see everything happening inside a player's computer. CS2AC judges the commands, settings, event subscriptions, shots, and movement visible to the server. It deliberately waits for detector-specific evidence instead of treating every unusual play as cheating.

</details>

## Quickstart

1. Install [Metamod:Source](https://www.sourcemm.net/) 2.x on a Windows x64 or Linux x64 CS2 dedicated server.
2. Download the matching CS2AC package from [GitHub Releases](https://github.com/karola3vax/CS2AC/releases).
3. Extract the archive into the CS2 server root without rearranging it. The archive begins with the `game` folder.
4. Edit `game/csgo/cfg/cs2ac.cfg`.
5. Start the server and run `meta list`, then `cs2ac_status`.

CS2AC loads its configuration automatically when the plugin starts and whenever a map begins.

## Configuration

The included [`cs2ac.cfg`](cfg/cs2ac.cfg) explains every setting in plain language. These are the settings most server owners will use:

| Setting | Default | Meaning |
| --- | --- | --- |
| `cs2ac_enabled` | `1` | Turn all detection on or off. |
| `cs2ac_whitelist` | empty | Comma-separated SteamID64s that may be detected but never punished. |
| `cs2ac_*_enabled` | `1` | Turn one detector on or off. |
| `cs2ac_chat_announcements` | `1` | Show public detection messages in chat. |
| `cs2ac_center_announcements` | `1` | Show the center-screen detection alert. |
| `cs2ac_punishment_command` | `css_addban ...` | Run this command for permanent-ban detections. |
| `cs2ac_kick_command` | `css_kick ...` | Run this command for kick-only detections. |
| `cs2ac_webhook_url` | empty | Send detection reports to this Discord webhook. |
| `cs2ac_webhook_role_id` | empty | Mention this Discord role in every report. |
| `cs2ac_webhook_server_address` | automatic | Override the public server address shown in reports. |
| `cs2ac_webhook_logo_url` | empty | Set the Discord branding and fallback avatar. |
| `cs2ac_allow_sv_cheats_testing` | `0` | Keep detectors active during deliberate local testing with `sv_cheats 1`. |

Punishment commands support `{steamid64}`, `{userid}`, and `{detection}`:

```cfg
cs2ac_punishment_command "css_addban {steamid64} 0 CS2AC: {detection}"
cs2ac_kick_command "css_kick #{userid} CS2AC: {detection}"
```

Whitelist one or more Steam accounts with:

```cfg
cs2ac_whitelist "76561198000000001,76561198000000002"
```

## Discord reports

1. Create a Discord webhook for the channel that should receive detections.
2. Put its URL in `cs2ac_webhook_url`.
3. Run `cs2ac_reload`.
4. Run `cs2ac_webhook_test`.

Keep the webhook URL private. If Discord returns an error, CS2AC disables further webhook attempts until the configuration is reloaded so a broken endpoint cannot keep delaying the server.

## Administrator commands

| Command | What it does |
| --- | --- |
| `cs2ac_status` | Show plugin, player, detector, announcement, punishment, and webhook status. |
| `cs2ac_help` | List the administrator commands. |
| `cs2ac_reload` | Reload and validate `cs2ac.cfg`. |
| `cs2ac_check_config` | Check the current settings without changing them. |
| `cs2ac_test_announcement` | Show a harmless chat and center-screen test. |
| `cs2ac_webhook_test` | Send a harmless Discord test report. |

## Honest limits

- CS2AC does not inspect client memory or files. The DLL Injection detector infers suspicious event subscriptions visible to the server.
- An administrator plugin is required if you want the configured kick and ban commands to do anything.
- Uncertain or incomplete evidence is rejected, which is safer but can allow some cheating to pass without a detection.
- Valve updates can change private engine details. Install a current build and check `cs2ac_status` after server updates.
- Debug settings are intended for short troubleshooting sessions and remain off by default.

## Building

Clone the pinned submodules with the project:

```sh
git clone --recursive https://github.com/karola3vax/CS2AC.git
cd CS2AC
```

Windows needs Python 3.8 or newer and Visual Studio 2022 with the C++ workload:

```powershell
./build-windows.ps1
```

Linux needs Python 3.8 or newer and Docker:

```sh
./build-linux.sh
```

Both scripts fetch the pinned AMBuild revision. Linux compilation runs inside the pinned Steam Runtime 3 SDK image. Directly installable files are placed under the build folder's `package/game` directory.

Pushes and pull requests build Windows and Linux in GitHub Actions. Pushing a signed or annotated `v*` tag creates a GitHub release with both packages.

## License

CS2AC is licensed under the [GNU Affero General Public License v3.0](LICENSE). Pinned SDK, build, and vendored dependencies keep their own licenses; see [Third-party notices](THIRD_PARTY_NOTICES.md). Binary packages include the applicable license texts.
