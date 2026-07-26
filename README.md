<div align="center">

<img src="docs/cs2ac-logo.png" width="760" alt="CS2AC">

### Open-source server-side anti-cheat for Counter-Strike 2.

[![Build](https://img.shields.io/github/actions/workflow/status/karola3vax/CS2AC/build.yml?branch=main&style=for-the-badge&label=build)](https://github.com/karola3vax/CS2AC/actions/workflows/build.yml)
[![Version](https://img.shields.io/badge/version-1.0.0-blue?style=for-the-badge)](https://github.com/karola3vax/CS2AC)
[![Detections](https://img.shields.io/badge/detections-17-red?style=for-the-badge)](#the-seventeen-detection-modules)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-5c7cfa?style=for-the-badge)](#quickstart)
[![License](https://img.shields.io/badge/license-AGPL--3.0-2ea44f?style=for-the-badge)](LICENSE)

**Counter-Strike is at its best when every shot, clutch, and win is earned.**

CS2AC helps community servers keep it that way.

[Install](#quickstart) · [See every detection](#the-seventeen-detection-modules) · [Pair it with CS2FOW](#want-wallhack-protection-too)

</div>

## See it catch

<table>
<tr>
<td width="50%" align="center">
<img src="docs/showcase/aimbot.gif" width="100%" alt="CS2AC detecting a blatant snap-hit aimbot">
<br><strong>AIMBOT</strong><br>
<sub>A blatant snap lands on target.</sub>
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

<div align="center">

### CS2AC + CS2FOW

<a href="https://github.com/karola3vax/CS2FOW">
<img src="docs/showcase/cs2fow-wallhack.gif" width="100%" alt="CS2FOW hiding enemies behind solid map geometry on Ancient">
</a>

**Catch the cheat. Starve the wallhack.**

<sub>CS2AC catches cheating behavior. CS2FOW stops hidden enemies from being sent to the cheater in the first place.</sub>

</div>

## The Seventeen Detection Modules

### Aim and accuracy

<table>
<tr>
<td width="22%"><strong>Aimbot</strong></td>
<td><strong>What it catches:</strong> The crosshair suddenly snapping onto an enemy before a shot.<br><strong>How it knows:</strong> The same sharp snap keeps ending in real damage.</td>
</tr>
<tr>
<td><strong>Aimlock</strong></td>
<td><strong>What it catches:</strong> Aim that follows a moving enemy with almost no shake or error.<br><strong>How it knows:</strong> The crosshair stays glued to the target for far too long, even through walls.</td>
</tr>
<tr>
<td><strong>Silentaim</strong></td>
<td><strong>What it catches:</strong> Bullets landing somewhere the crosshair never appeared to point.<br><strong>How it knows:</strong> The player's aim, the bullet impact, and the damage do not line up.</td>
</tr>
<tr>
<td><strong>Inhuman Accuracy</strong></td>
<td><strong>What it catches:</strong> Long streaks of accuracy with almost none of the misses real players make.<br><strong>How it knows:</strong> The impossible hit rate continues across a full set of aimed shots.</td>
</tr>
<tr>
<td><strong>Irregular Behavior</strong></td>
<td><strong>What it catches:</strong> Airborne and no-scope kills happening far too reliably.<br><strong>How it knows:</strong> It counts the misses too, so one lucky shot means nothing.</td>
</tr>
</table>

### Movement

<table>
<tr>
<td width="22%"><strong>Autostrafe</strong></td>
<td><strong>What it catches:</strong> Automatic air strafes that preserve speed and make every movement count.<br><strong>How it knows:</strong> Jump after jump is too fast, efficient, and perfectly timed.</td>
</tr>
<tr>
<td><strong>Bhop</strong></td>
<td><strong>What it catches:</strong> Jump scripts that fire at the exact moment the player lands.<br><strong>How it knows:</strong> Frame-perfect hops or the same landing pattern keep repeating.</td>
</tr>
<tr>
<td><strong>Hyperscroll</strong></td>
<td><strong>What it catches:</strong> Far more jump presses than normal mouse-wheel scrolling can explain.<br><strong>How it knows:</strong> The excessive scrolling also keeps producing perfect hops.</td>
</tr>
<tr>
<td><strong>Nulls</strong></td>
<td><strong>What it catches:</strong> Opposite movement keys switching perfectly while the player is airborne.<br><strong>How it knows:</strong> The direction changes stay too perfect for too long.</td>
</tr>
</table>

### Exploits and client behavior

<table>
<tr>
<td width="22%"><strong>Antiaim</strong></td>
<td><strong>What it catches:</strong> Fake angles that make the player spin, jitter, or look in impossible directions.<br><strong>How it knows:</strong> The view becomes impossible or jumps away for a shot before snapping straight back.</td>
</tr>
<tr>
<td><strong>DLL Injection</strong></td>
<td><strong>What it catches:</strong> Injected cheats listening to hidden game events.<br><strong>How it knows:</strong> The player's game starts listening to events linked to cheat features.</td>
</tr>
<tr>
<td><strong>Desubticking</strong></td>
<td><strong>What it catches:</strong> Automated movement with its normal timing stripped away.<br><strong>How it knows:</strong> Movement keeps arriving at the exact same instant instead of naturally between ticks.</td>
</tr>
<tr>
<td><strong>Doubletap</strong></td>
<td><strong>What it catches:</strong> The same weapon firing twice before it should be ready.<br><strong>How it knows:</strong> The two shots are faster than that weapon can normally fire.</td>
</tr>
<tr>
<td><strong>Invalid CVar</strong></td>
<td><strong>What it catches:</strong> Protected game settings changed to values normal players should never have.<br><strong>How it knows:</strong> The player's game reports a setting that is unsafe or impossible.</td>
</tr>
<tr>
<td><strong>Invalid Input</strong></td>
<td><strong>What it catches:</strong> Movement buttons changing without the normal record of those presses.<br><strong>How it knows:</strong> The button presses and the player's movement tell two different stories.</td>
</tr>
<tr>
<td><strong>Namechanger</strong></td>
<td><strong>What it catches:</strong> Rapid name changes used to distract people or show off.<br><strong>How it knows:</strong> The same player changes name five times within one minute.</td>
</tr>
<tr>
<td><strong>Subtick Spam</strong></td>
<td><strong>What it catches:</strong> Too many movement and aim changes packed into the same instant.<br><strong>How it knows:</strong> Those unnatural bursts keep repeating within a very short time.</td>
</tr>
</table>

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

1. Open this repository's **Releases** tab and choose the matching Windows or Linux package.
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

## FAQ

<details>
<summary><strong>Do players install anything?</strong></summary>

Nope. CS2AC lives entirely on the dedicated server, so players just connect and play as usual.

</details>

<details>
<summary><strong>Does it work in Premier or Valve matchmaking?</strong></summary>

No. CS2AC is made for community and dedicated servers you control. It cannot be added to Premier or Valve matchmaking.

</details>

<details>
<summary><strong>Can it catch every cheat?</strong></summary>

No. No anti-cheat catches everything. CS2AC can only judge the behavior that reaches the server; it does not read a player's files, memory, or desktop.

Despite the name, **DLL Injection does not scan anyone's PC**. It only checks suspicious game-event subscriptions that the client shares with the server.

</details>

<details>
<summary><strong>Which detections ban and which only kick?</strong></summary>

By default, Desubticking, Nulls, and Subtick Spam only kick. Every other detection uses the permanent-ban command.

Want different punishments? Change or empty either command. The detection announcements will keep working.

</details>

<details>
<summary><strong>What happens to whitelisted players?</strong></summary>

They can still trigger a detection, and everyone can still see it, but CS2AC stops before sending any punishment command.

</details>

<details>
<summary><strong>Does it support FFA?</strong></summary>

Yes. When `mp_teammates_are_enemies` is enabled, CS2AC treats other players as enemies just like the game does.

</details>

<details>
<summary><strong>Does CS2AC advertise itself?</strong></summary>

Yes, but it does not spam. Every six completed rounds, CS2AC shows this message once in chat and at the center of the screen:

```text
[CS2AC] This server is protected by karola3vax's anti-cheat.
```

This small project credit is built in and cannot be turned off.

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
