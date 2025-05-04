# AFM_PapaBear

**AFM_PapaBear** is a tribute to the iconic *Papa Bear* radio operator from *Operation Flashpoint*, designed to give Arma Reforger gamemasters and scenario creators a powerful tool for in-game communication. It allows you to easily send immersive radio-style messages to players during missions. Additionally, the addon supports automated broadcasts, enabling number station-style transmissions for atmospheric or narrative purposes.


## Features

- Messages are sent over radio and also with ICONIC UPPER CASE STYLE OVER CHAT.
- Supports scripted or scheduled broadcasts for number station behavior.
- Immersive enhancement for both PvP and PvE scenarios.

## Usage

1. Add the AFM_PapaBearRadio or AFM_NumberRadioStation entity to game world
2. In the ActionsManagerComponent define the exact message you want to send
3. Trigger the transmission ingame

## Development

1. Install Arma Reforger Tools and Enfusion Workbench via Steam.
2. Open the Workbench and load the AFM_PapaBear project.
3. Core files are located in:
   - `Scripts/Game/AFM_PapaBear/`
   - `Sounds/Samples`


## Contributing

We welcome community contributions!

To contribute:

1. Create a new branch for your changes.
2. Submit a pull request with a clear description of what you’ve changed or added.

If you're reporting a bug or requesting a feature, please open a GitHub Issue with a detailed description.


## Missing features

- Prefabs aren't currently available to gamemaster, they have to be placed using enfusion editor
- Triggering transmission is done via player action, I should probably add a GM module that does this


## Credits

- Developed by the [ArmaForces](https://github.com/ArmaForces) team.
- Inspired by the original *Papa Bear* from **Operation Flashpoint: Cold War Crisis**, I have trained the voice model on the original game samples. All credist go to Bohemia Interactive

## License

This project is licensed under the [Arma Public License (APL)](https://www.bohemia.net/community/licenses/arma-public-license).