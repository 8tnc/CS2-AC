<div align="center">

<img src="docs/cs2ac-logo.png" width="760" alt="CS2AC">

### Open-source server-side anti-cheat for Counter-Strike 2.

[![Build](https://img.shields.io/github/actions/workflow/status/karola3vax/CS2AC/build.yml?branch=main&style=for-the-badge&label=build)](https://github.com/karola3vax/CS2AC/actions/workflows/build.yml)
[![Version](https://img.shields.io/badge/version-1.0.0-blue?style=for-the-badge)](https://github.com/karola3vax/CS2AC)
[![Detections](https://img.shields.io/badge/detections-17-red?style=for-the-badge)](#17-detections-one-plugin)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-5c7cfa?style=for-the-badge)](#quickstart)
[![License](https://img.shields.io/badge/license-AGPL--3.0-2ea44f?style=for-the-badge)](LICENSE)

**Counter-Strike is at its best when every shot, clutch, and win is earned.**

CS2AC helps community servers keep it that way.

Server-side only. Nothing for players to install. Just join and play.

[Download](https://github.com/karola3vax/CS2AC/releases) · [Install](#quickstart) · [See every detection](#17-detections-one-plugin) · [Pair it with CS2FOW](#want-wallhack-protection-too)

</div>

Some cheats rage. Others try to look human. CS2AC watches the aim, shots, movement, and inputs that reach the server, turns suspicious behavior into evidence, and acts when a detector reaches its threshold.

<table>
<tr>
<td width="50%"><strong>17 focused detections</strong><br>Aim cheats, movement assists, exploits, and tampered clients.</td>
<td width="50%"><strong>Visible when it matters</strong><br>Detections appear in public chat and on screen.</td>
</tr>
<tr>
<td width="50%"><strong>Evidence in Discord</strong><br>See the player, SteamID64, reason, server, map, and punishment.</td>
<td width="50%"><strong>Works with your admin plugin</strong><br>Run any ban or kick command your server already understands.</td>
</tr>
<tr>
<td width="50%"><strong>Whitelist without silence</strong><br>Trusted players are still detected and announced, but never punished.</td>
<td width="50%"><strong>Built for Metamod</strong><br>Runs on Windows and Linux with no CounterStrikeSharp dependency.</td>
</tr>
</table>

## See it catch

Real in-game clips. No mockups.

<table>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/aimbot.gif" width="100%" alt="CS2AC detecting a blatant snap-hit aimbot">
<br><strong>AIMBOT</strong><br>
<sub>A blatant snap lands on target. CS2AC keeps the evidence.</sub>
</td>
<td width="50%" align="center">
<img src="docs/showcase/aimlock.gif" width="100%" alt="CS2AC detecting inhumanly precise target tracking">
<br><strong>AIMLOCK</strong><br>
<sub>The crosshair follows a moving target with inhuman precision.</sub>
</td>
</tr>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/antiaim.gif" width="100%" alt="CS2AC detecting impossible anti-aim angles">
<br><strong>ANTIAIM</strong><br>
<sub>Impossible angles, attack-return, jitter, and spin patterns.</sub>
</td>
<td width="50%" align="center">
<img src="docs/showcase/bhop.gif" width="100%" alt="CS2AC detecting automated bunny hopping">
<br><strong>BHOP</strong><br>
<sub>Repeated frame-perfect hops and machine-like jump patterns.</sub>
</td>
</tr>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/irregular-behavior.gif" width="100%" alt="CS2AC detecting repeated irregular airborne and no-scope results">
<br><strong>IRREGULAR BEHAVIOR</strong><br>
<sub>Too many rage-level airborne and no-scope results.</sub>
</td>
<td width="50%" align="center">
<img src="docs/showcase/silentaim.gif" width="100%" alt="CS2AC detecting bullets that disagree with visible aim">
<br><strong>SILENTAIM</strong><br>
<sub>The bullets hit somewhere the visible aim never pointed.</sub>
</td>
</tr>
</table>

## 17 detections. One plugin.

### Aim and accuracy

- **Aimbot** — The aim snaps onto an enemy just before a damaging shot.
- **Aimlock** — The aim follows a moving enemy with machine-like precision for too long, even through walls.
- **Silentaim** — A damaging bullet lands far away from where the player's aim was pointing.
- **Inhuman Accuracy** — A long run of shots lands with accuracy that normal play cannot sustain.
- **Irregular Behavior** — The player repeatedly lands highly unlikely shots while airborne or without scoping.

### Movement

- **Autostrafe** — Air strafes repeat faster and more efficiently than a player can perform by hand.
- **Bhop** — Too many jumps land and take off again with frame-perfect timing.
- **Hyperscroll** — Jump inputs repeat too quickly and consistently for normal mouse-wheel scrolling.
- **Nulls** — Opposite movement keys switch with perfectly automated timing while airborne.

### Exploits and client behavior

- **Antiaim** — View angles spin, snap back after shots, or reach positions a normal client cannot produce.
- **DLL Injection** — The player's game listens to hidden events commonly used by injected cheat DLLs.
- **Desubticking** — Movement inputs repeatedly arrive without their normal between-tick timing.
- **Doubletap** — The same weapon fires twice sooner than the game normally allows.
- **Invalid CVar** — Protected or safety-critical settings use values a normal client should never have.
- **Invalid Input** — Pressed and released buttons disagree with the input history sent by the client.
- **Namechanger** — The player changes their visible name five times within one minute.
- **Subtick Spam** — One game tick is flooded with repeated movement and angle changes.

## One detection. Everywhere.

When CS2AC acts, it can do all of this at once:

1. Announce the detection in public chat.
2. Hold a clear center-screen alert for five seconds.
3. Write the evidence and punishment result to the server console.
4. Run your configured ban or kick command.
5. Send a detailed Discord webhook report.

```text
[CS2AC] detected AIMBOT on Player and punished.
```

<div align="center">

<img src="docs/showcase/announcement-chat.png" width="600" alt="CS2AC test announcement in public chat">

<table>
<tr>
<td width="33%" align="center">
<img src="docs/showcase/announcement-center.png" width="100%" alt="CS2AC center-screen test announcement">
<br><strong>Five-second center alert</strong>
</td>
<td width="33%" align="center">
<img src="docs/showcase/detection.png" width="100%" alt="CS2AC center-screen Aimbot detection">
<br><strong>Detection sent</strong>
</td>
<td width="33%" align="center">
<img src="docs/showcase/whitelist.png" width="100%" alt="CS2AC announcing a detection on a whitelisted player">
<br><strong>Whitelist stays visible</strong>
</td>
</tr>
</table>

</div>

Whitelisting does not silence CS2AC. The detection still appears in chat, on screen, in the console, and in Discord; only the punishment command is skipped.

## Quickstart

You need a Windows x64 or Linux x64 CS2 dedicated server running [Metamod:Source](https://www.sourcemm.net/) 2.x.

1. Download the matching package from [GitHub Releases](https://github.com/karola3vax/CS2AC/releases).
2. Extract it into the CS2 server root without rearranging anything. The package begins with the `game` folder.
3. Edit `game/csgo/cfg/cs2ac.cfg`.
4. Start the server.
5. Run `meta list`, then `cs2ac_status`.

That is it. Players install nothing.

The default punishment commands are made for [CS2-SimpleAdmin](https://github.com/daffyyyy/CS2-SimpleAdmin). Using another admin plugin? Replace the two commands in `cs2ac.cfg` with commands that plugin understands.

## Configuration that makes sense

The included [`cs2ac.cfg`](cfg/cs2ac.cfg) explains every option in plain language.

| Setting | Default | What it does |
| --- | ---: | --- |
| `cs2ac_enabled` | `1` | Master switch for CS2AC. |
| `cs2ac_whitelist` | empty | SteamID64s that may be detected but must never be punished. |
| `cs2ac_*_enabled` | `1` | Enable or disable one detection module. |
| `cs2ac_chat_announcements` | `1` | Show detections in public chat. |
| `cs2ac_center_announcements` | `1` | Show the five-second center alert. |
| `cs2ac_punishment_command` | `css_addban ...` | Command used for permanent bans. |
| `cs2ac_kick_command` | `css_kick ...` | Command used for kick-only detections. |
| `cs2ac_webhook_url` | empty | Discord webhook that receives detection reports. |
| `cs2ac_webhook_role_id` | empty | Discord role to mention on a report. |
| `cs2ac_webhook_server_address` | automatic | Server address shown in Discord. |
| `cs2ac_allow_sv_cheats_testing` | `0` | Allow local detector testing with `sv_cheats 1`. Never enable this on a public server. |

Punishment commands support `{steamid64}`, `{userid}`, and `{detection}`:

```cfg
cs2ac_punishment_command "css_addban {steamid64} 0 CS2AC: {detection}"
cs2ac_kick_command "css_kick #{userid} CS2AC: {detection}"
```

Whitelist one account or a comma-separated list:

```cfg
cs2ac_whitelist "76561198000000001,76561198000000002"
```

### Discord in four steps

1. Create a webhook in the Discord channel that should receive detections.
2. Put its URL in `cs2ac_webhook_url`.
3. Run `cs2ac_reload`.
4. Run `cs2ac_webhook_test`.

Keep the webhook URL private. CS2AC never prints it back to the console.

<div align="center">
<img src="docs/showcase/webhook.png" width="432" alt="CS2AC Discord detection report with player, evidence, punishment, map, and server details">
</div>

<details>
<summary><strong>Server commands</strong></summary>

| Command | What it does |
| --- | --- |
| `cs2ac_status` | Show whether CS2AC and its main features are working. |
| `cs2ac_help` | List the available CS2AC commands. |
| `cs2ac_reload` | Reload `cs2ac.cfg`. |
| `cs2ac_check_config` | Find mistakes in the current configuration. |
| `cs2ac_test_announcement` | Preview the chat and center-screen alert without detecting anyone. |
| `cs2ac_webhook_test` | Send a test detection report to Discord. |

</details>

## Straight answers

<details>
<summary><strong>Do players install anything?</strong></summary>

No. CS2AC runs inside the dedicated server. Players connect and play normally.

</details>

<details>
<summary><strong>Does it work in Premier or Valve matchmaking?</strong></summary>

No. You need control of a community or dedicated server running Metamod:Source.

</details>

<details>
<summary><strong>Can it catch every cheat?</strong></summary>

No anti-cheat can promise that. CS2AC sees the cheating behavior that reaches the server; it cannot inspect a player's files, memory, or desktop.

That also means **DLL Injection does not scan anyone's PC**. It looks for suspicious game-event subscriptions that the client exposes to the server.

</details>

<details>
<summary><strong>Which detections ban and which only kick?</strong></summary>

Desubticking, Nulls, and Subtick Spam use the kick command by default. Every other detection uses the permanent-ban command by default.

You can replace or empty either command without disabling detection announcements.

</details>

<details>
<summary><strong>What happens to whitelisted players?</strong></summary>

They are still detected and shown everywhere, but CS2AC never sends a punishment command for them.

</details>

<details>
<summary><strong>Does it support FFA?</strong></summary>

Yes. CS2AC follows `mp_teammates_are_enemies`, so other players are treated as enemies when FFA is enabled.

</details>

<details>
<summary><strong>Does CS2AC advertise itself?</strong></summary>

Every six completed rounds, CS2AC shows one chat and center-screen credit:

```text
[CS2AC] This server is protected by karola3vax's anti-cheat.
```

This project credit is built in and has no disable option.

</details>

## Want wallhack protection too?

**CS2AC catches cheating behavior. [CS2FOW](https://github.com/karola3vax/CS2FOW) stops your server from sending live enemy positions through solid walls and smoke.**

They solve different problems, run entirely on the server, and can protect the same CS2 community server together.

## Building from source

Clone the pinned submodules:

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

Both scripts make a directly installable package under the build folder's `package/game` directory. Linux builds inside the pinned Steam Runtime 3 SDK image.

## Be part of it

Run CS2AC on a real server. Test it. Break it. Send useful reports.

If it earns a place on your server, **star the repository and share your clips**. That is how an open-source anti-cheat gets harder to bypass.

## License

CS2AC is free and open-source software licensed under the [GNU Affero General Public License v3.0](LICENSE). Dependencies keep their own licenses; see [Third-party notices](THIRD_PARTY_NOTICES.md).
