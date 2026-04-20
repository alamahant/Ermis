#include"helpmenudialog.h"
#include <QApplication>
#include<QString>
HelpMenuDialog::HelpMenuDialog(HelpType type, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(getTitle(type));
    setModal(true);
    resize(600, 500);

    setupUI();
    loadContent(type);
}

void HelpMenuDialog::setupUI()
{
    m_layout = new QVBoxLayout(this);

    m_contentArea = new QTextEdit(this);
    m_contentArea->setReadOnly(true);
   m_contentArea->setAcceptRichText(true);


    m_closeButton = new QPushButton("Close", this);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);

    m_layout->addWidget(m_contentArea);
    m_layout->addWidget(m_closeButton);
}

void HelpMenuDialog::loadContent(HelpType type)
{
    QString content;

    switch (type) {
    case HelpType::About:
        content = getAboutContent();
        break;
    case HelpType::Features:
        content = getFeaturesContent();
        break;
    case HelpType::Instructions:
        content = getInstructionsContent();
        break;
    case HelpType::BestPractices:
        content = getBestPracticesContent();
        break;
    case HelpType::WhatsNew:
        content = getWhatsNewContent();
        break;
    case HelpType::Shortcuts:
        content = getShortcutsContent();
        break;
    case HelpType::Security:
        content = getSecurityContent();
        break;
    case HelpType::TwoFA:
        content = getTwoFAContent();
        break;
    case HelpType::DataManagement:
        content = getDataManagementContent();
        break;
    case HelpType::DownloadManagement:
        content = getDownloadManagerContent();
        break;
    case HelpType::onSitesAndSessions:
        content = getSitesSessionsContent();
        break;
    case HelpType::onSecurity:
        content = getOnSecurityContent();
        break;
    case HelpType::onNewStorageSystem:
        content = getOnNewStorageSystemContent();
        break;
    case HelpType::onNamedProfiles:
        content = getOnNamedProfilesContent();
        break;
    case HelpType::onChangelog:
        content = getChangelogContent();
        break;
    case HelpType::supportUs:
        content = getSupportusContent();
        break;
    }


    m_contentArea->setHtml(content);
}

QString HelpMenuDialog::getTitle(HelpType type)
{
    switch (type) {
    case HelpType::About: return "About BinauralPlayer";
    case HelpType::Features: return "Features";
    case HelpType::Instructions: return "Instructions";
    case HelpType::BestPractices: return "Best Practices";
    case HelpType::WhatsNew: return "What's New";
    case HelpType::Shortcuts: return "Keyboard Shortcuts";
    case HelpType::Security: return "Jasmine Security";
    case HelpType::TwoFA: return "Jasmine 2FA Utility";
    case HelpType::DataManagement: return "Jasmine Data Management";
    case HelpType::DownloadManagement: return "Jasmine Download Management";
    case HelpType::onSitesAndSessions: return "On Sites And Sessions";
    case HelpType::onSecurity: return "On Security";
    case HelpType::onNewStorageSystem: return "On the New Storage System";
    case HelpType::onNamedProfiles: return "On Shared Named Profiles";
    case HelpType::onChangelog: return "Changelog";

    case HelpType::supportUs: return "Support Us";

    default: return "Help";
    }
}

QString HelpMenuDialog::getBestPracticesContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">

            <h1 style="color: #8e44ad; text-align: center; margin-bottom: 20px;">
                🧠 Best Practices for Binaural & Isochronic Audio
            </h1>

            <p>
                Brainwave entrainment is the practice of using rhythmic audio—such as binaural beats
                or isochronic tones—to gently guide the brain into specific states of relaxation,
                focus, meditation, or sleep. While extremely helpful when used correctly, it’s important
                to understand how it works and how to use it safely.
            </p>

            <h2 style="color: #3498db;">🌊 What Are Binaural Beats?</h2>
            <p>
                Binaural beats occur when you play slightly different frequencies in each ear.
                The brain detects the difference between the tones and begins synchronizing with
                the resulting “beat frequency.” This effect requires <strong>headphones</strong>.
            </p>

            <h2 style="color: #3498db;">⚡ What Are Isochronic Tones?</h2>
            <p>
                Isochronic tones are single, sharply-pulsed tones that turn on and off at a steady rhythm.
                They do *not* require headphones and are considered one of the strongest forms of
                brainwave entrainment.
            </p>

            <h2 style="color: #27ae60;">✨ Potential Benefits</h2>
            <ul style="padding-left: 20px;">
                <li>Deep relaxation and meditation assistance</li>
                <li>Improved focus and concentration</li>
                <li>Increased creativity and problem-solving</li>
                <li>Enhanced sleep and dream clarity</li>
                <li>Stress and anxiety reduction</li>
                <li>Mood balancing and emotional grounding</li>
            </ul>

            <h2 style="color: #c0392b;">⚠️ Safety & Dangers</h2>
            <ul style="padding-left: 20px;">
                <li>Do <strong>not</strong> use while driving or operating machinery</li>
                <li>Avoid high-intensity frequencies if you are prone to seizures or epilepsy</li>
                <li>Start with short sessions (10–15 minutes) before longer ones</li>
                <li>If you feel dizziness or discomfort, stop immediately</li>
                <li>Use comfortable volume levels—entrainment does not require loud audio</li>
                <li>Never force meditation states; let the brain shift naturally</li>
            </ul>

            <h2 style="color: #2980b9;">📜 Useful Frequency Lists</h2>
            <p>
                A large list of brainwave-related frequencies can be found here:
            </p>
            <ul style="padding-left: 20px;">
                <li><a href="https://www.lunarsight.com/freq.htm">https://www.lunarsight.com/freq.htm</a></li>
                <li><a href="https://docs.preterhuman.net/Brainwave/Cymatic_Frequency_Listing">https://docs.preterhuman.net/Brainwave/Cymatic_Frequency_Listing</a></li>
            </ul>

            <h2 style="color: #16a085;">🌿 Background Ambience</h2>
            <p>
                Atmospheric nature sounds greatly enhance the experience. High-quality,
                royalty-free ambient audio can be found at:
            </p>
            <p>
                <a href="https://pixabay.com/sound-effects/search/nature/">https://pixabay.com/sound-effects/search/nature/</a>
            </p>

            <h2 style="color: #8e44ad;">💡 Best Practices</h2>
            <ul style="padding-left: 20px;">
                <li>Use good headphones for binaural beats</li>
                <li>Use speakers or headphones for isochronic tones</li>
                <li>Meditate or sit comfortably during sessions</li>
                <li>Combine tones with relaxing ambience or maybe some relaxxing music for deeper immersion</li>
                <li>Choose frequencies appropriate for your goal (sleep, focus, creativity, etc.)</li>
                <li>Consistency matters—small daily sessions are better than long sporadic ones</li>
            </ul>

            <div style="background-color: #f8f9fa; padding: 15px; border-radius: 8px; margin-top: 25px; text-align: center;">
                <strong>
                    Brainwave entrainment is a powerful tool—use it mindfully, with intention,
                    and always listen to your body.
                </strong>
            </div>

        </div>
    )";
}


QString HelpMenuDialog::getWhatsNewContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #8e44ad; text-align: center; margin-bottom: 20px;">🌳 What's New - Dynamic Audio Engine & Enhanced Features</h1>

            <div style="background-color: #f0f0f5; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #8e44ad; margin-top: 0;">⚡ Major Feature Additions - Version 1.3.0</h2>

                <div style="background-color: #e8f4f8; padding: 15px; border-radius: 8px; margin: 15px 0; border-left: 4px solid #3498db;">
                    <h3 style="color: #3498db; margin-top: 0;">🎵 Multi-Stage Session Manager</h3>
                    <p>A sophisticated system for creating and executing timed sequences of audio tones. Users define multiple stages with parameters (binaural/isochronic tones, frequencies, waveforms, duration, volume) that play sequentially.</p>
                    <ul>
                        <li><strong>Text-based stage definition</strong> using simple colon-separated format</li>
                        <li><strong>Three tone types</strong>: Binaural (beat frequencies), Isochronic (pulsed), Generator (mono)</li>
                        <li><strong>Full session control</strong>: Play, pause, stop, save/load sessions</li>
                        <li><strong>Real-time visual feedback</strong>: Current stage highlighting, time remaining display</li>
                        <li><strong>Auto-transitions</strong> between stages with volume fading</li>
                        <li><strong>File support</strong>: Save/load sessions as .txt or .bsession files</li>
                        <li><strong>Parameter validation</strong> with error messages</li>
                        <li><strong>Timer synchronization</strong> between stage and total session time</li>
                    </ul>
                    <p><strong>User Workflow:</strong> Enter stages → Parse/validate → Save → Play → Monitor progress with visual highlighting</p>
                </div>

                <div style="background-color: #f0f8ff; padding: 15px; border-radius: 8px; margin: 15px 0; border-left: 4px solid #9b59b6;">
                    <h3 style="color: #9b59b6; margin-top: 0;">📀 CUE Sheet Import Feature</h3>
                    <p>A tool for importing and navigating audio tracks defined in CUE sheet files. Parses standard CUE files to extract track information and enables direct playback of individual tracks within long audio files.</p>
                    <ul>
                        <li><strong>Load standard CUE files</strong> (.cue format) with associated audio files</li>
                        <li><strong>Track list display</strong> showing track numbers, titles, performers, and start times</li>
                        <li><strong>Direct track access</strong> via double-click or play button</li>
                        <li><strong>Navigation controls</strong>: Previous/Next track buttons</li>
                        <li><strong>Relative path handling</strong> automatically resolves audio file locations</li>
                        <li><strong>User confirmation dialog</strong> before loading parsed tracks</li>
                        <li><strong>Integration with audio player</strong> - sends exact start positions to main application</li>
                    </ul>
                    <p><strong>User Workflow:</strong> Load CUE file → View track list → Select track → Play from precise start position</p>
                </div>

                <div style="background-color: #fff8e1; padding: 15px; border-radius: 8px; margin: 15px 0; border-left: 4px solid #e67e22;">
                    <h3 style="color: #e67e22; margin-top: 0;">⏱️ Digital Seek Widget</h3>
                    <p>A precision navigation tool allowing users to jump to specific time positions within audio tracks.</p>
                    <ul>
                        <li>Enter exact time values in minutes:seconds format</li>
                        <li>Instantly seek to precise positions in the current track</li>
                        <li>Fine-grained control over playback location</li>
                        <li>Quick navigation without manual scrolling</li>
                    </ul>
                </div>

                <div style="background-color: #f0fff0; padding: 15px; border-radius: 8px; margin: 15px 0; border-left: 4px solid #2E8B57;">
                    <h3 style="color: #2E8B57; margin-top: 0;">📂 Drag-and-Drop File Import</h3>
                    <p>Streamlined file import functionality for enhanced workflow efficiency.</p>
                    <ul>
                        <li>Drag music files directly from your file system into the application</li>
                        <li>Supports various audio formats</li>
                        <li>Quick loading without traditional file dialogs</li>
                        <li>Intuitive file management</li>
                    </ul>
                </div>
            </div>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">⚡ BinauralPlayer Goes Fully Dynamic - Version 1.2.0</h2>
                <p>Tone generation has been switched to a real-time dynamic engine.</p>
                <ul>
                    <li>Buffered tone generation removed entirely.</li>
                    <li>No delays or gaps when increasing or decreasing frequencies — changes are applied instantly.</li>
                </ul>
                <p>This milestone opens the road to new features:</p>
                <ul>
                    <li>Programmable multi-stage sessions</li>
                    <li>Per-stage frequency changes</li>
                    <li>Support for binaural beats and isochronic pulses within complex session flows</li>
                </ul>
            </div>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Introducing the Ambient Sound Control System - Version 1.1.0</h2>
                <p>A powerful new feature that lets you create and control layered nature soundscapes
                for relaxation, focus, meditation, and atmospheric enhancement.</p>
            </div>

            <h2 style="color: #2E8B57; border-bottom: 2px solid #2E8B57; padding-bottom: 5px;">🌿 Nature Toolbar Features</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #2E8B57;">🎚️ 5-Channel Ambient Mixer</h3>
                <p>Five independent sound players that can run simultaneously. Create complex sound environments
                by mixing rain, forest, thunder, ocean, and more.</p>

                <h3 style="color: #2E8B57;">🔘 Global Power Control</h3>
                <p>Single power button to enable/disable all nature sounds at once. Perfect for quick mute
                or instant atmosphere activation.</p>

                <h3 style="color: #2E8B57;">⏯️ Master Transport Controls</h3>
                <p>Play, pause, and stop ALL active nature sounds simultaneously. Color-coded buttons provide
                clear visual feedback: green (play), orange (pause), red (stop).</p>

                <h3 style="color: #2E8B57;">🎛️ Individual Player Buttons</h3>
                <p>Each channel has its own button showing name, state, and status. Single-click toggles
                play/pause. Color indicates status: green (playing), orange (paused), gray (disabled).</p>

                <h3 style="color: #2E8B57;">📊 Master Volume Control</h3>
                <p>Slider controls overall ambient sound level with perceptual volume curve for natural
                human hearing response.</p>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">💾 Preset System</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #9b59b6;">💽 Save Configurations</h3>
                <p>Store complete sound setups including audio files, volumes, names, and enabled states.</p>

                <h3 style="color: #9b59b6;">📂 Load Presets</h3>
                <p>Quickly switch between saved environments like "Rainy Forest", "Ocean Waves", or "Focus Study".</p>

                <h3 style="color: #9b59b6;">🔄 Reset Function</h3>
                <p>One-click restoration to default settings with confirmation dialog for safety.</p>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">🎵 Player Capabilities</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #e67e22;">🎧 Multi-Format Audio</h3>
                <p>Supports MP3, WAV, OGG, FLAC, and M4A files with integrated file browser.</p>

                <h3 style="color: #e67e22;">🔤 Custom Naming</h3>
                <p>Give each channel descriptive names up to 10 characters (e.g., "Rain", "Birds", "Thunder").</p>

                <h3 style="color: #e67e22;">🔁 Auto-Repeat</h3>
                <p>Loop audio files seamlessly for continuous background ambience.</p>

                <h3 style="color: #e67e22;">📏 Individual Volume</h3>
                <p>Fine-tune each channel's volume independently (0-100%).</p>

                <h3 style="color: #e67e22;">⏱️ Progress Tracking</h3>
                <p>See playback progress with seek capability in detailed settings dialog.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🖥️ Interface Design</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #3498db;">🎨 Non-Destructive Dialogs</h3>
                <p>Player settings dialogs hide instead of close, preserving your configuration state.</p>

                <h3 style="color: #3498db;">👁️ Visual Status Indicators</h3>
                <p>Buttons show real-time status with icons and colors for instant recognition.</p>

                <h3 style="color: #3498db;">🔍 Tooltip Guidance</h3>
                <p>All controls have descriptive tooltips explaining their function.</p>

                <h3 style="color: #3498db;">📱 Responsive Layout</h3>
                <p>Clean toolbar design that integrates seamlessly with existing interface.</p>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🚀 Quick Start Guide</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #1abc9c;">🎯 Enhanced Workflow Integration</h3>
                <p>Combine all new features for powerful audio experiences:</p>
                <ol style="margin-left: 20px; padding-left: 0;">
                    <li><strong>Drag-and-drop</strong> music files directly into the application</li>
                    <li><strong>Load CUE sheets</strong> for structured album navigation</li>
                    <li><strong>Use digital seek</strong> for precise time jumping within tracks</li>
                    <li><strong>Create multi-stage sessions</strong> for programmable tone sequences</li>
                    <li><strong>Mix with ambient sounds</strong> for layered audio environments</li>
                </ol>

                <h3 style="color: #1abc9c;">⚡ Getting Started</h3>
                <ol style="margin-left: 20px; padding-left: 0;">
                    <li><strong>Power On</strong>: Click the ○ button to activate the system</li>
                    <li><strong>Add Sounds</strong>: Click any player button (P1-P5) to open settings</li>
                    <li><strong>Browse Files</strong>: Select nature sound files for each channel</li>
                    <li><strong>Customize Names</strong>: Give descriptive names to each channel</li>
                    <li><strong>Adjust Volumes</strong>: Set levels to create your perfect mix</li>
                    <li><strong>Control Playback</strong>: Use master controls or click individual buttons</li>
                </ol>

                <h3 style="color: #1abc9c;">🎵 Download Nature Sounds</h3>
                <p>Get high-quality, royalty-free ambient sounds from:</p>
                <div style="background-color: #f0f9ff; padding: 15px; border-radius: 8px; border-left: 4px solid #3498db; margin: 15px 0;">
                    <p><strong>🌐 Pixabay Sound Effects:</strong> <a href="https://pixabay.com/sound-effects/search/nature/" style="color: #3498db; text-decoration: none;">https://pixabay.com/sound-effects/search/nature/</a></p>
                    <p>After downloading, place your audio files in:</p>
                    <div style="background-color: #e8f4f8; padding: 10px; border-radius: 6px; font-family: monospace; margin: 10px 0;">
                        ~/Documents/BinauralPlayer/ambient-tracks/
                    </div>
                    <p>Recommended downloads:</p>
                    <ul style="margin-left: 20px; padding-left: 0;">
                        <li>Rain & Thunderstorms</li>
                        <li>Forest & Birds</li>
                        <li>Ocean Waves</li>
                        <li>Streams & Rivers</li>
                        <li>Wind & Nature Ambience</li>
                    </ul>
                    <p style="font-style: italic; margin-top: 10px;">💡 <strong>Tip:</strong> The ambient player will automatically look for files in this directory when browsing!</p>
                </div>

                <h3 style="color: #1abc9c;">💡 Pro Tips</h3>
                <ul style="margin-left: 20px; padding-left: 0;">
                    <li>Start with 2-3 complementary sounds for best results</li>
                    <li>Save your favorite mixes as presets for quick access</li>
                    <li>Use the power button for instant silence during calls</li>
                    <li>Combine binaural beats with ambient sounds for enhanced meditation</li>
                    <li>Create multi-stage sessions for guided audio therapy sequences</li>
                    <li>Use CUE sheets for seamless album or long-track navigation</li>
                </ul>
            </div>

            <div style="background-color: #f8f9fa; padding: 20px; border-radius: 8px; text-align: center; margin-top: 30px;">
                <h3 style="color: #2c3e50; margin-top: 0;">🌟 Transform Your Audio Environment</h3>
                <p style="margin-bottom: 0;">BinauralPlayer now offers comprehensive audio control: from dynamic tone generation and programmable multi-stage sessions to ambient sound mixing and precise file navigation. Create your perfect audio environment for relaxation, focus, meditation, or therapeutic purposes with our complete toolkit!</p>
            </div>
        </div>
    )";
}


QString HelpMenuDialog::getShortcutsContent()
{
    return QString();  // TODO: Implement
}

QString HelpMenuDialog::getAboutContent() {
    return QString(R"(
        <div style="text-align: center; font-family: Arial, sans-serif;">
            <h1 style="color: #2c3e50; margin-bottom: 10px;">🕊️ Ermis</h1>
            <h3 style="color: #7f8c8d; margin-bottom: 20px;">Steganography & Covert Communication Tool</h3>
            <p style="font-size: 16px; margin-bottom: 20px;">
                A cross-platform steganography application for hiding and extracting secret data within
                digital media files. Named after the Greek god of messages and communication, Ermis enables
                covert communication through innocent-looking image and audio files.
            </p>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin: 20px 0;">
                <p style="margin: 5px 0;"><strong>Version:</strong> %1</p>
                <p style="margin: 5px 0;"><strong>Built with:</strong> Qt Framework (Qt 6) + FFmpeg</p>
                <p style="margin: 5px 0;"><strong>Platform:</strong> Cross-platform</p>
            </div>

            <div style="margin: 30px 0;">
                <h4 style="color: #2c3e50;">Key Features</h4>

                <p style="text-align: left; margin: 10px 20px;">
                    • <strong>Image Steganography</strong> — Hide data in PNG, JPG, BMP using LSB techniques<br>
                    • <strong>Audio Steganography</strong> — Support for WAV, MP3, FLAC, OGG with FFmpeg conversion<br>
                    • <strong>Dual Input Modes</strong> — Hide text messages or entire files with filename preservation<br>
                    • <strong>AES Encryption</strong> — Optional passphrase protection with "ENCR" marker detection<br>
                    • <strong>PRT Artistic Mode</strong> — Specialized visual steganography with camera scanning<br>
                    • <strong>Drag & Drop</strong> — Simply drag files directly into the application<br>
                    • <strong>Smart Extraction</strong> — Auto-detects encryption, file type, and text content<br>
                    • <strong>Live Capacity Indicator</strong> — Real-time feedback on available hiding space
                </p>

                <p>Available for Linux, Windows, and macOS.</p>
                <p>
                    Project Repository
                    <br>
                    <a href="https://github.com/alamahant/Ermis">https://github.com/alamahant/Ermis</a>
                </p>
            </div>

            <hr style="margin: 30px 0; border: 1px solid #bdc3c7;">

            <div style="margin: 20px 0; background-color: #f8f9fa; padding: 15px; border-radius: 8px;">
                <h4 style="color: #2c3e50; margin-bottom: 10px;">🌐 Network Steganography</h4>
                <p style="text-align: left; margin: 10px 20px;">
                    • <strong>ERTP Protocol</strong> — Ermis Reliable Transfer Protocol for covert network communication<br>
                    • <strong>ICMP (Ping)</strong> — Hide data within ICMP echo request/reply packets<br>
                    • <strong>UDP</strong> — Covert data transmission over UDP packets with custom payloads<br>
                    • <strong>DNS</strong> — Encoded messages within DNS queries and responses<br>
                    • <strong>HTTP/HTTPS (TLS)</strong> — Steganography within HTTP headers and TLS encrypted tunnels<br>
                    • Reliable delivery with error correction and packet sequencing<br>
                    • Bypasses traditional firewall detection by mimicking normal network traffic<br>
                    • Perfect for secure, undetectable data transfer over networks
                </p>
                <p style="margin-top: 10px;">
                    <strong>ERTP Protocol Repository:</strong><br>
                    <a href="https://github.com/alamahant/ERTP">https://github.com/alamahant/ERTP</a>
                </p>
            </div>

            <hr style="margin: 30px 0; border: 1px solid #bdc3c7;">

            <div style="margin: 20px 0;">
                <p style="margin: 5px 0; color: #7f8c8d;">
                    <strong>Copyright © 2026 Dharma</strong>
                </p>
                <p style="margin: 5px 0; font-size: 12px; color: #95a5a6;">
                    All rights reserved. This software is provided as-is without warranty.
                </p>
            </div>

            <div style="margin: 20px 0;">
                <p style="font-size: 14px; color: #7f8c8d;">
                    Crafted with ❤️ for privacy advocates, digital artists, and security enthusiasts
                </p>
            </div>
        </div>
    )").arg(QApplication::applicationVersion());
}

QString HelpMenuDialog::getFeaturesContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #8e44ad; text-align: center; margin-bottom: 20px;">Ermis Features</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Steganography & Covert Communication</h2>
                <p>Cross-platform steganography combining image, audio, text, PDF, distributed, and network hiding with optional encryption for private multi-layer communication.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">Image Steganography</h2>
            <div style="margin: 20px 0;">
                <h3 style="color: #e67e22;">Multi-Format Support</h3>
                <p>Hide data in PNG, JPG, BMP using LSB (Least Significant Bit) techniques.</p>
                <h3 style="color: #e67e22;">Live Preview</h3>
                <p>Side-by-side comparison of original and modified images with zoom and pan.</p>
                <h3 style="color: #e67e22;">Capacity Indicator</h3>
                <p>Real-time available space display in bytes and percentage.</p>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">Audio Steganography</h2>
            <div style="margin: 20px 0;">
                <h3 style="color: #9b59b6;">Wide Format Support</h3>
                <p>WAV, MP3, FLAC, OGG audio files with automatic FFmpeg conversion.</p>
                <h3 style="color: #9b59b6;">Metadata Extraction</h3>
                <p>Automatic sample rate, channels, bit depth, and duration detection.</p>
                <h3 style="color: #9b59b6;">Audio Playback</h3>
                <p>Built-in player with play/pause/stop and volume controls.</p>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">Text Steganography</h2>
            <div style="margin: 20px 0;">
                <h3 style="color: #27ae60;">Zero-Width Character Encoding</h3>
                <p>Hide messages in plain text using invisible Unicode characters (U+200B, U+200C).</p>
                <h3 style="color: #27ae60;">Binary-to-Text Mapping</h3>
                <p>1 bit per visible character with 4-byte header for data integrity.</p>
                <h3 style="color: #27ae60;">Cover Text Support</h3>
                <p>Hide in existing text or generate capacity-aware cover messages.</p>
                <h3 style="color: #27ae60;">Real-Time Capacity</h3>
                <p>Calculate available space from cover text length with percentage indicator.</p>
            </div>

            <h2 style="color: #2980b9; border-bottom: 2px solid #2980b9; padding-bottom: 5px;">PDF Steganography</h2>
            <div style="margin: 20px 0;">
                <h3 style="color: #2980b9;">Object Stream Embedding</h3>
                <p>Hide data inside PDF files using /ObjStm object streams with FlateDecode compression.</p>
                <h3 style="color: #2980b9;">Incremental Updates</h3>
                <p>Spec-compliant appending preserves PDF structure for broad viewer compatibility.</p>
                <h3 style="color: #2980b9;">Filename Preservation</h3>
                <p>FNAM flag stores original filename for seamless file reconstruction.</p>
                <h3 style="color: #2980b9;">Capacity Management</h3>
                <p>Conservative ~50% carrier size limit prevents suspicious file bloat.</p>
            </div>

            <h2 style="color: #d35400; border-bottom: 2px solid #d35400; padding-bottom: 5px;">Distributed Steganography</h2>
            <div style="margin: 20px 0;">
                <h3 style="color: #d35400;">Wikipedia Pointer Maps</h3>
                <p>Hide messages as word positions across Wikipedia articles - no single carrier file.</p>
                <h3 style="color: #d35400;">JSON Map Format</h3>
                <p>Portable pointer maps with language wrapper for cross-platform reconstruction.</p>
                <h3 style="color: #d35400;">Multi-Language Support</h3>
                <p>English, German, French, Spanish, Italian, Portuguese, Dutch, Russian, Japanese, Chinese.</p>
                <h3 style="color: #d35400;">Rate-Limited API</h3>
                <p>Automatic throttling respects Wikipedia API limits with progress tracking.</p>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">Network Steganography (ERTP)</h2>
            <div style="margin: 20px 0;">
                <h3 style="color: #8e44ad;">Multi-Protocol Support</h3>
                <p>ICMP Echo/Reply, DNS over UDP, raw UDP, and HTTP/TLS packet embedding.</p>
                <h3 style="color: #8e44ad;">Reliable Delivery</h3>
                <p>Sliding window protocol with ACK/NACK, CRC32 checksums, automatic retransmission.</p>
                <h3 style="color: #8e44ad;">Session Management</h3>
                <p>32-bit unique IDs, configurable timeouts (1000-2000ms), 3-5 retry attempts.</p>
                <h3 style="color: #8e44ad;">IP Filtering</h3>
                <p>Whitelist/blacklist with CIDR notation support for source validation.</p>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">Data Handling</h2>
            <div style="margin: 20px 0;">
                <h3 style="color: #e74c3c;">Text & File Input</h3>
                <p>Hide plain text or any file type with automatic filename preservation (FNAM flag).</p>
                <h3 style="color: #e74c3c;">Smart Detection</h3>
                <p>Automatic extraction detects text, files, binary, or encrypted data (ENCR flag).</p>
                <h3 style="color: #e74c3c;">Binary Auto-Save</h3>
                <p>Extracted binary files prompt for save with timestamp_ermis_filename format.</p>
                <h3 style="color: #e74c3c;">Format Validation</h3>
                <p>Auto-detect text vs binary with warnings for incompatible formats.</p>
            </div>

            <h2 style="color: #c0392b; border-bottom: 2px solid #c0392b; padding-bottom: 5px;">Security Features</h2>
            <div style="margin: 20px 0;">
                <h3 style="color: #c0392b;">AES-256-CBC Encryption</h3>
                <p>PBKDF2 key derivation (100,000 iterations) with random 16-byte salt.</p>
                <h3 style="color: #c0392b;">Multi-Layer Security</h3>
                <p>Compose any steg method + encryption + network steg + TLS for layered protection.</p>
                <h3 style="color: #c0392b;">Passphrase Memory</h3>
                <p>Session-based passphrase caching for convenience.</p>
                <h3 style="color: #c0392b;">Auto Encryption Detection</h3>
                <p>ENCR flag triggers automatic passphrase prompt on extraction.</p>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">User Interface</h2>
            <div style="margin: 20px 0;">
                <h3 style="color: #1abc9c;">Multi-Tab Interface</h3>
                <p>Separate tabs for hide/extract operations across all steganography modes.</p>
                <h3 style="color: #1abc9c;">Theme Support</h3>
                <p>Dark and light themes with persistent preference across sessions.</p>
                <h3 style="color: #1abc9c;">Info Buttons</h3>
                <p>Theme-aware corner widgets with comprehensive feature documentation.</p>
                <h3 style="color: #1abc9c;">Drag & Drop</h3>
                <p>Direct file input for images, audio, PDFs, and text files.</p>
                <h3 style="color: #1abc9c;">Clipboard Integration</h3>
                <p>One-click copy/paste for text and extracted data.</p>
                <h3 style="color: #1abc9c;">Progress Tracking</h3>
                <p>Real-time operation feedback with status messages.</p>
            </div>

            <h2 style="color: #16a085; border-bottom: 2px solid #16a085; padding-bottom: 5px;">Technical Highlights</h2>
            <div style="margin: 20px 0;">
                <h3 style="color: #16a085;">Modular Architecture</h3>
                <p>Separate engines: image, audio, text, PDF, distributed, and network protocols.</p>
                <h3 style="color: #16a085;">FFmpeg Integration</h3>
                <p>Professional audio conversion and metadata extraction.</p>
                <h3 style="color: #16a085;">Multi-Threaded Operations</h3>
                <p>Non-blocking file I/O with progress callbacks.</p>
                <h3 style="color: #16a085;">Cross-Platform</h3>
                <p>Linux, Windows, macOS support with smart directory fallbacks.</p>
                <h3 style="color: #16a085;">Timestamped Output</h3>
                <p>Automatic unique filenames for stego and extracted files.</p>
            </div>

            <div style="background-color: #f8f9fa; padding: 20px; border-radius: 8px; text-align: center; margin-top: 30px;">
                <h3 style="color: #2c3e50; margin-top: 0;">Silent Messenger</h3>
                <p style="margin-bottom: 0;">Ermis combines classical steganography with modern media support, PDF embedding, distributed pointer maps, and network protocols, enabling multi-layer covert communication through innocent-looking files and network traffic.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getInstructionsContent()
{
    return R"(
<div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
    <h1 style="color: #3498db; text-align: center; margin-bottom: 20px;">How to Use Ermis</h1>

    <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
        <h2 style="color: #2c3e50; margin-top: 0;">Overview</h2>
        <p>Ermis allows you to hide secret data inside innocent-looking image, audio, text, PDF, and network files, and extract hidden data from such files. The application features six main steganography modes: Image, Audio, Text, PDF, Distributed (Wikipedia pointer maps), and Network (ERTP Protocol). Each dialog includes an info button (ⓘ) in the top-right corner with feature-specific guidance.</p>
    </div>

    <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">Hiding Data in Images</h2>

    <div style="margin: 20px 0;">
        <h3 style="color: #e67e22;">Step 1: Select Carrier Image</h3>
        <p>Click "Select Carrier Image" or drag and drop an image file (PNG, JPG, BMP) into the application. The image will appear in the left preview panel.</p>

        <h3 style="color: #e67e22;">Step 2: Choose Data Type</h3>
        <p>Select either Text Input to type your secret message, or File Input to browse and select any file to hide (documents, images, etc.).</p>

        <h3 style="color: #e67e22;">Step 3: Optional Encryption</h3>
        <p>Check "Encrypt data" to protect your hidden content with a passphrase. Your content will be encrypted with AES-256 before being hidden in the image.</p>

        <h3 style="color: #e67e22;">Step 4: Check Capacity</h3>
        <p>The capacity indicator shows how much data can be hidden in the current image. Ensure your data size does not exceed the available space.</p>

        <h3 style="color: #e67e22;">Step 5: Hide Data</h3>
        <p>Click "Hide Data" to embed your information. The modified image will appear in the right preview panel.</p>

        <h3 style="color: #e67e22;">Step 6: Save Stego Image</h3>
        <p>Click "Save Modified Image" to save the image with hidden data. PNG format is recommended for best results.</p>
    </div>

    <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">Hiding Data in Audio</h2>

    <div style="margin: 20px 0;">
        <h3 style="color: #e67e22;">Step 1: Enable Audio Mode</h3>
        <p>Check the "AUDIO" checkbox in the top-right corner to switch to audio steganography mode.</p>

        <h3 style="color: #e67e22;">Step 2: Select Audio Carrier</h3>
        <p>Click "Select Audio Carrier" or drag and drop an audio file (MP3, WAV, FLAC, OGG). Non-WAV files are automatically converted to WAV format.</p>

        <h3 style="color: #e67e22;">Step 3-6: Same as Image Mode</h3>
        <p>Follow the same steps for data input, encryption, and hiding. The modified audio can be saved as a WAV file.</p>

        <div style="background-color: #e8f8f5; padding: 15px; border-radius: 8px; margin: 15px 0;">
            <h3 style="color: #117a65; margin-top: 0;">Audio Playback</h3>
            <p>When an audio file is loaded, the built-in audio player appears at the bottom with controls for Play/Pause/Stop, volume slider, progress bar, and time display.</p>
        </div>
    </div>

    <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">Hiding Data in Text</h2>

    <div style="margin: 20px 0;">
        <h3 style="color: #e67e22;">Step 1: Enter Cover Text</h3>
        <p>Load or paste normal-looking text that will serve as the cover. This can be any paragraph, article, or document.</p>

        <h3 style="color: #e67e22;">Step 2: Choose Secret Data</h3>
        <p>Select Text Input to type a message, or File Input to hide any file type within the cover text.</p>

        <h3 style="color: #e67e22;">Step 3: Check Capacity</h3>
        <p>The capacity calculator shows available space. Capacity is approximately 1 bit per visible character in the cover text.</p>

        <h3 style="color: #e67e22;">Step 4: Optional Encryption</h3>
        <p>Check "Encrypt data" to encrypt your content before hiding it in the text.</p>

        <h3 style="color: #e67e22;">Step 5: Hide Data</h3>
        <p>Click "Hide Data" to embed your content using zero-width Unicode characters (U+200B, U+200C). These characters are invisible to readers but contain your secret data.</p>

        <h3 style="color: #e67e22;">Step 6: Save or Copy</h3>
        <p>Save the text-with-hidden-data to a file or copy directly to clipboard for sharing.</p>
    </div>

    <h2 style="color: #2980b9; border-bottom: 2px solid #2980b9; padding-bottom: 5px;">PDF Steganography</h2>

    <div style="margin: 20px 0;">
        <h3 style="color: #e67e22;">Overview</h3>
        <p>Hide any file or text inside PDF documents using object stream embedding. The hidden data is compressed and stored as a PDF object, preserving the original filename for binary files.</p>

        <h3 style="color: #e67e22;">Step 1: Select Secret Data</h3>
        <p>Choose Text tab to type a message, or File tab to select any file. The original filename is preserved for file mode.</p>

        <h3 style="color: #e67e22;">Step 2: Select Carrier PDF</h3>
        <p>Click "Browse..." to select a PDF file to hide data in. Capacity is approximately 50% of the PDF file size.</p>

        <h3 style="color: #e67e22;">Step 3: Choose Output Location</h3>
        <p>Specify where to save the stego PDF. A default name with "_stego" suffix is suggested.</p>

        <h3 style="color: #e67e22;">Step 4: Optional Encryption</h3>
        <p>Check "Encrypt data before hiding" to protect content with a passphrase. Both data and filename are encrypted.</p>

        <h3 style="color: #e67e22;">Step 5: Embed Data</h3>
        <p>Click "Embed Data" to create the stego PDF. Progress bar shows embedding status.</p>
    </div>

    <h2 style="color: #d35400; border-bottom: 2px solid #d35400; padding-bottom: 5px;">Distributed Steganography</h2>

    <div style="margin: 20px 0;">
        <h3 style="color: #e67e22;">Overview</h3>
        <p>Hide messages across Wikipedia articles using word position pointers. No single carrier file exists - the message is reconstructed from article word positions.</p>

        <h3 style="color: #e67e22;">Step 1: Enter Secret Message</h3>
        <p>Type your message in the text area. Only letters, numbers, and spaces are allowed.</p>

        <h3 style="color: #e67e22;">Step 2: Select Wikipedia Language</h3>
        <p>Choose from 10 supported languages (English default).</p>

        <h3 style="color: #e67e22;">Step 3: Optional Encryption</h3>
        <p>Check "Encrypt JSON Pointer Map" to protect the output with a passphrase.</p>

        <h3 style="color: #e67e22;">Step 4: Build Pointer Map</h3>
        <p>Click "Build Pointer Map" to generate the JSON map. Progress shows words mapped.</p>

        <h3 style="color: #e67e22;">Step 5: Save or Share Map</h3>
        <p>Save as .json (plain) or .txt (encrypted). Share this small map file instead of a large carrier.</p>
    </div>

    <h2 style="color: #c0392b; border-bottom: 2px solid #c0392b; padding-bottom: 5px;">Network Steganography (ERTP)</h2>

    <div style="margin: 20px 0;">
        <h3 style="color: #e67e22;">Overview</h3>
        <p>ERTP enables sending hidden data through network packets using ICMP, DNS, UDP, or HTTP/TLS protocols.</p>

        <h3 style="color: #e67e22;">Critical: Pre-Connection Setup</h3>
        <p style="background-color: #ffebee; border-left: 4px solid #e74c3c; padding: 10px; color: #b71c1c;"><b>Before connecting:</b> Peers must agree beforehand on the protocol (ICMP/DNS/UDP/HTTPS) and port number. Sender must also be listening to receive ACKs. Without listening, transmission will fail.</p>

        <h3 style="color: #e67e22;">Step 1: Choose Protocol</h3>
        <p>Select ICMP, DNS, UDP, or HTTP/TLS. Both sender and receiver must use the same protocol and port.</p>

        <h3 style="color: #e67e22;">Step 2: Enter Target</h3>
        <p>Enter the target IP address. Set port number if using UDP/DNS/HTTP.</p>

        <h3 style="color: #e67e22;">Step 3: Select Data</h3>
        <p>Choose Text Input for messages or File Input for any file type.</p>

        <h3 style="color: #e67e22;">Step 4: Start Listening</h3>
        <p style="background-color: #ffebee; border-left: 4px solid #e74c3c; padding: 10px; color: #b71c1c;">Both sender and receiver must click "Start Listening" on the Receive tab before transmission.</p>

        <h3 style="color: #e67e22;">Step 5: Send Data</h3>
        <p>Click "Send" to transmit. Progress tracking shows packet transfer status.</p>
    </div>

    <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">Extracting Hidden Data</h2>

    <div style="margin: 20px 0;">
        <h3 style="color: #e67e22;">Step 1: Load Stego File</h3>
        <p>In the "Extract Data" tab, load the file containing hidden data (image, audio, text, or PDF).</p>

        <h3 style="color: #e67e22;">Step 2: Extract Data</h3>
        <p>Click "Extract Data". If encrypted content is detected (ENCR flag), passphrase prompt appears automatically.</p>

        <h3 style="color: #e67e22;">Step 3: Save Extracted Data</h3>
        <p>Text displays in the viewer. Binary files prompt for save location with original filename preserved.</p>
    </div>

    <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">Info Buttons</h2>

    <div style="margin: 20px 0;">
        <p>Each steganography dialog includes an info button (ⓘ) in the top-right corner of the tab bar. Click it to view:</p>
        <ul style="padding-left: 20px;">
            <li>Feature-specific instructions and workflow</li>
            <li>Capacity limits and format support</li>
            <li>Encryption options and security notes</li>
            <li>Important warnings and coordination requirements</li>
        </ul>
        <p>The button adapts to dark/light themes and provides quick reference without leaving the dialog.</p>
    </div>

    <div style="background-color: #ffebee; border: 2px solid #f44336; padding: 20px; border-radius: 8px; margin: 25px 0;">
        <h3 style="color: #c62828; text-align: center; margin-top: 0;">Legal and Ethical Notice</h3>
        <p style="text-align: center; color: #b71c1c;">Ermis is designed for legitimate purposes only.</p>
        <ul style="padding-left: 20px; color: #b71c1c;">
            <li>Privacy: Protect your personal information</li>
            <li>Digital watermarking: Mark your creative works</li>
            <li>Educational use: Learn about steganography techniques</li>
            <li>Authorized communication only: Respect laws and regulations</li>
        </ul>
        <p style="margin-bottom: 0; color: #b71c1c;">Users are solely responsible for compliance with applicable laws in their jurisdiction.</p>
    </div>

    <div style="background-color: #f8f9fa; padding: 20px; border-radius: 8px; text-align: center; margin-top: 30px;">
        <h3 style="color: #2c3e50; margin-top: 0;">Quick Tips</h3>
        <ul style="text-align: left; padding-left: 40px;">
            <li>Always test extraction before relying on hidden data</li>
            <li>Use PNG for images as it preserves hidden data perfectly</li>
            <li>Remember passphrases - lost passphrases cannot recover encrypted content</li>
            <li>Check capacity first to avoid wasting time on files too small</li>
            <li>File names are preserved when hiding files - recovered automatically</li>
            <li>PDF steganography works with any PDF - capacity is ~50% of file size</li>
            <li>Distributed maps are small JSON files - easy to share via any channel</li>
            <li>Network steganography requires both peers to listen on same protocol/port</li>
            <li>Click the ⓘ info button in any dialog for feature-specific guidance</li>
        </ul>
    </div>
</div>
    )";
}

QString HelpMenuDialog::getSecurityContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">🔐 Security Features</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Master Password Protection</h2>
                <p>Jasmine includes a comprehensive security system to protect your saved websites, sessions, and sensitive data.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🛡️ Password Protection Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Master password requirement on startup</li>
                    <li>Secure password hashing with salt encryption</li>
                    <li>Failed attempt protection (5 attempts maximum)</li>
                    <li>Factory reset option for forgotten passwords</li>
                </ul>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">🔧 How to Enable Password Protection</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Go to Security menu → "Require Password on Startup"</li>
                    <li>Read the security notice and click OK</li>
                    <li>Enter your new master password</li>
                    <li>Confirm your password</li>
                    <li>Jasmine will now require this password on every startup</li>
                </ol>
            </div>

            <div style="background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;">
                <h3 style="color: #856404; margin-top: 0;">⚠️ Important Security Notes</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Choose a strong, memorable password</li>
                    <li>Write it down in a safe place</li>
                    <li>If you forget it, you'll need to factory reset</li>
                    <li>Password is encrypted and stored securely</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">🔄 Changing Your Master Password</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Go to Security menu → "Change Master Password"</li>
                    <li>Enter your new password</li>
                    <li>Confirm the new password</li>
                    <li>Password is updated immediately</li>
                </ol>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">🚨 Failed Login Protection</h2>

            <div style="margin: 20px 0;">
                <p><strong>Maximum 5 password attempts allowed</strong></p>
                <p>After 5 failed attempts, you get two options:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Exit Application</strong></li>
                    <li><strong>Factory Reset</strong> (clears all data and security settings)</li>
                </ul>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🔄 Factory Reset</h2>

            <div style="margin: 20px 0;">
                <p>If you forget your master password, factory reset will:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Remove all security settings</li>
                    <li>Clear the master password</li>
                    <li>Reset password protection to disabled</li>
                    <li>Allow you to start fresh</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">⚙️ Security Menu Options</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>"Require Password on Startup"</strong> - Toggle password protection on/off</li>
                    <li><strong>"Change Master Password"</strong> - Update your existing password</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">🔒 Protection States</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #d35400;">When Password Protection is Active:</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Jasmine prompts for password on every startup</li>
                    <li>All your websites, sessions, and data remain encrypted</li>
                    <li>No access to application features without correct password</li>
                </ul>

                <h3 style="color: #d35400;">When Password Protection is Disabled:</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Jasmine starts immediately without password prompt</li>
                    <li>All features accessible without authentication</li>
                    <li>Data remains saved but unprotected</li>
                </ul>
            </div>

            <h2 style="color: #16a085; border-bottom: 2px solid #16a085; padding-bottom: 5px;">💡 Best Practices</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Use a unique password not used elsewhere</li>
                    <li>Include numbers, letters, and special characters</li>
                    <li>Avoid easily guessable information</li>
                    <li>Keep a secure backup of your password</li>
                    <li>Enable password protection if you store sensitive login information</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">🔐 Security Implementation</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>SHA-256 encryption with custom salt</li>
                    <li>No plain text password storage</li>
                    <li>Secure settings storage</li>
                    <li>Memory-safe password handling</li>
                </ul>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🛡️ Complete Protection</h3>
                <p style="margin-bottom: 0;">This security system ensures your browsing profiles, saved websites, sessions, and any stored login references remain protected even if someone gains access to your computer.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getTwoFAContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">🔐 2FA Code Generator</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Two-Factor Authentication Manager</h2>
                <p>Jasmine includes a built-in Two-Factor Authentication (2FA) code generator that helps you manage and generate time-based one-time passwords (TOTP) for your accounts.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">❓ What is 2FA?</h2>

            <div style="margin: 20px 0;">
                <p>Two-Factor Authentication adds an extra layer of security to your accounts by requiring a second form of verification beyond just your password. This usually involves a 6-digit code that changes every 30 seconds.</p>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">🚀 Key Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Generate 6-digit TOTP codes for any 2FA-enabled account</li>
                    <li>Real-time code updates every 30 seconds</li>
                    <li>Visual countdown timer showing when codes refresh</li>
                    <li>One-click code copying to clipboard</li>
                    <li>Secure local storage of account secrets</li>
                    <li>Support for multiple accounts from different services</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">📱 How to Access</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Open the 2FA Manager from the Toolbar icon or the Tools Menu</li>
                    <li>The manager opens in a separate window</li>
                    <li>Resizable interface with accounts list and code display</li>
                </ul>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">➕ Adding 2FA Accounts</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Click "Add Account" button</li>
                    <li>Enter account name (e.g., "GitHub", "Google", "Discord")</li>
                    <li>Paste the secret key from the website's 2FA setup</li>
                    <li>Optionally enter the issuer/company name</li>
                    <li>Click OK to save</li>
                </ol>
            </div>

            <div style="background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;">
                <h3 style="color: #856404; margin-top: 0;">🔍 Where to Find Secret Keys</h3>
                <p>When enabling 2FA on websites, they typically show:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>A QR code for mobile apps</li>
                    <li>A text secret key (what you need for Jasmine)</li>
                    <li>Look for "Can't scan QR code?" or "Manual entry" options</li>
                </ul>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">🔢 Using Generated Codes</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Select an account from the list</li>
                    <li>Current 6-digit code displays in large text</li>
                    <li>Countdown timer shows seconds until next refresh</li>
                    <li>Click "Copy Code to Clipboard" for easy pasting</li>
                    <li>Codes automatically update every 30 seconds</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">👁️ Visual Indicators</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Progress bar shows time remaining (green → yellow → red)</li>
                    <li>Large, easy-to-read monospace font for codes</li>
                    <li>Clear countdown text showing refresh time</li>
                    <li>Copy button changes to "Copied!" for confirmation</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">⚙️ Account Management</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>View all your 2FA accounts in organized list</li>
                    <li>Delete accounts you no longer need</li>
                    <li>Accounts persist between application restarts</li>
                    <li>Secure local storage (not cloud-synced)</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">🔒 Security Notes</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Secret keys are stored locally on your device</li>
                    <li>No internet connection required for code generation</li>
                    <li>Codes are generated using industry-standard TOTP algorithm</li>
                    <li>Same codes as Google Authenticator, Authy, etc.</li>
                </ul>
            </div>

            <h2 style="color: #16a085; border-bottom: 2px solid #16a085; padding-bottom: 5px;">🌐 Supported Services</h2>

            <div style="margin: 20px 0;">
                <p>Works with any service that supports TOTP 2FA:</p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Google/Gmail accounts</li>
                    <li>GitHub</li>
                    <li>Discord</li>
                    <li>Microsoft accounts</li>
                    <li>Banking websites</li>
                    <li>Social media platforms</li>
                    <li>And many more</li>
                </ul>
            </div>

            <h2 style="color: #d35400; border-bottom: 2px solid #d35400; padding-bottom: 5px;">📋 Workflow Example</h2>

            <div style="margin: 20px 0;">
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Enable 2FA on GitHub</li>
                    <li>Copy the secret key from GitHub's setup page</li>
                    <li>Add account in Jasmine's 2FA Manager</li>
                    <li>When logging into GitHub, select the account</li>
                    <li>Copy the current 6-digit code</li>
                    <li>Paste into GitHub's 2FA prompt</li>
                </ol>
            </div>

            <h2 style="color: #7b1fa2; border-bottom: 2px solid #7b1fa2; padding-bottom: 5px;">💡 Benefits Over Mobile Apps</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Access codes directly on your computer</li>
                    <li>No need to grab your phone</li>
                    <li>Larger, easier-to-read display</li>
                    <li>Integrated with your browsing workflow</li>
                    <li>Quick clipboard copying</li>
                </ul>
            </div>

            <h2 style="color: #388e3c; border-bottom: 2px solid #388e3c; padding-bottom: 5px;">⏰ Time Synchronization</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Codes are time-based (30-second intervals)</li>
                    <li>Uses your system clock for accuracy</li>
                    <li>Same timing as other authenticator apps</li>
                    <li>Automatic refresh every second</li>
                </ul>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🌟 Integrated Security</h3>
                <p style="margin-bottom: 0;">This 2FA manager eliminates the need for separate authenticator apps while providing the same security benefits, making it convenient to access your two-factor codes directly within Jasmine.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getDataManagementContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">🗂️ Data Management & Privacy</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Session & Profile Data Management</h2>
                <p>Jasmine provides comprehensive tools to manage your browsing data, sessions, and privacy settings. Control what data is stored and when to clear it.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🧹 Clean Current Session Data</h2>

            <div style="margin: 20px 0;">
                <p><strong>What it does:</strong></p>
                <p>Clears browsing data from all currently active sessions and the shared profile.</p>

                <p><strong>Data removed:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All cookies from active sessions</li>
                    <li>HTTP cache from all profiles</li>
                    <li>Visited links history</li>
                    <li>Temporary browsing data</li>
                </ul>

                <p><strong>When to use:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>After browsing sensitive websites</li>
                    <li>When sharing your computer</li>
                    <li>To free up storage space</li>
                    <li>For privacy after online shopping/banking</li>
                </ul>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">👥 Clean Shared Profile Data</h2>

            <div style="margin: 20px 0;">
                <p><strong>What it does:</strong></p>
                <p>Clears browsing data only from the shared profile, leaving separate tab profiles untouched.</p>

                <p><strong>Data removed:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Shared profile cookies only</li>
                    <li>Shared profile cache</li>
                    <li>Shared profile visited links</li>
                </ul>

                <p><strong>What's preserved:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Individual tab profile data</li>
                    <li>Private profile sessions</li>
                    <li>Separate profile cookies and cache</li>
                </ul>

                <p><strong>When to use:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>When you want to keep private profiles intact</li>
                    <li>To clear general browsing without affecting work profiles</li>
                    <li>Selective privacy cleaning</li>
                </ul>
            </div>

            <h2 style="color: #e74c3c; border-bottom: 2px solid #e74c3c; padding-bottom: 5px;">🏭 Restore Factory Defaults</h2>

            <div style="margin: 20px 0;">
                <p><strong>What it does:</strong></p>
                <p>Completely resets Jasmine to its original state, removing all user data and settings.</p>

                <p><strong>Data removed:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All saved websites and bookmarks</li>
                    <li>All saved sessions</li>
                    <li>All application settings and preferences</li>
                    <li>Security settings and master passwords</li>
                    <li>All browsing data (cookies, cache, history)</li>
                    <li>Application data directories</li>
                    <li>Profile configurations</li>
                </ul>
            </div>

            <div style="background-color: #f8d7da; padding: 15px; border-radius: 8px; margin: 15px 0; border-left: 4px solid #dc3545;">
                <h3 style="color: #721c24; margin-top: 0;">⚠️ Factory Reset Warning</h3>
                <p style="margin-bottom: 0;"><strong>This action cannot be undone!</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All your saved data will be permanently lost</li>
                    <li>Application will close automatically after reset</li>
                    <li>You'll need to restart Jasmine manually</li>
                    <li>All customizations will be lost</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">📍 How to Access These Features</h2>

            <div style="margin: 20px 0;">
                <p>All data management options are located in the <strong>Sessions</strong> menu:</p>
                <ol style="margin: 10px 0; padding-left: 25px;">
                    <li>Click on "Sessions" in the menu bar</li>
                    <li>Scroll to the bottom section</li>
                    <li>Choose your desired cleaning option</li>
                    <li>Confirm the action in the dialog box</li>
                </ol>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🔄 Data Types Explained</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #8e44ad;">Cookies</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Login sessions and preferences</li>
                    <li>Shopping cart contents</li>
                    <li>Website customizations</li>
                </ul>

                <h3 style="color: #8e44ad;">HTTP Cache</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Temporarily stored website files</li>
                    <li>Images, scripts, and stylesheets</li>
                    <li>Speeds up repeat visits</li>
                </ul>

                <h3 style="color: #8e44ad;">Visited Links</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>History of visited websites</li>
                    <li>Link color changes (visited vs unvisited)</li>
                    <li>Navigation history</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🛡️ Privacy Recommendations</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #16a085;">Regular Cleaning (Weekly)</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Use "Clean Shared Profile Data" for routine maintenance</li>
                    <li>Keeps private profiles intact</li>
                    <li>Maintains good performance</li>
                </ul>

                <h3 style="color: #16a085;">Deep Cleaning (Monthly)</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Use "Clean Current Session Data" for thorough cleanup</li>
                    <li>Clears all active session data</li>
                    <li>Good for privacy and storage</li>
                </ul>

                <h3 style="color: #16a085;">Emergency Cleaning</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>After using public computers</li>
                    <li>When selling or giving away device</li>
                    <li>After security concerns</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">💡 Smart Usage Tips</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Before cleaning:</strong> Save any important sessions you want to keep</li>
                    <li><strong>Profile separation:</strong> Use private profiles for sensitive browsing</li>
                    <li><strong>Regular maintenance:</strong> Clean shared profile weekly, all data monthly</li>
                    <li><strong>Factory reset:</strong> Only use when starting completely fresh</li>
                    <li><strong>Backup important data:</strong> Export sessions before major cleaning</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">⚡ Performance Benefits</h2>

            <div style="margin: 20px 0;">
                <p><strong>Regular data cleaning provides:</strong></p>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Faster application startup</li>
                    <li>Reduced memory usage</li>
                    <li>More available storage space</li>
                    <li>Improved browsing performance</li>
                    <li>Better privacy protection</li>
                </ul>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🎯 Choose the Right Tool</h3>
                <p style="margin-bottom: 0;">
                    <strong>Shared Profile Clean:</strong> For routine maintenance<br>
                    <strong>Current Session Clean:</strong> For thorough privacy cleaning<br>
                    <strong>Factory Reset:</strong> For complete fresh start
                </p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getDownloadManagerContent()
{
    return R"(
        <div style="font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;">
            <h1 style="color: #e74c3c; text-align: center; margin-bottom: 20px;">📥 Download Manager</h1>

            <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin-bottom: 25px;">
                <h2 style="color: #2c3e50; margin-top: 0;">Integrated Download Management</h2>
                <p>Jasmine includes a comprehensive download manager that handles all your file downloads with progress tracking, organization, and easy access to downloaded files.</p>
            </div>

            <h2 style="color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px;">🚀 Key Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Real-time download progress tracking</li>
                    <li>Download speed and time remaining calculations</li>
                    <li>Automatic file organization in dedicated folder</li>
                    <li>Duplicate filename handling</li>
                    <li>One-click access to files and folders</li>
                    <li>Download history management</li>
                    <li>Cancel active downloads</li>
                    <li>Clean interface with visual progress bars</li>
                </ul>
            </div>

            <h2 style="color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px;">📍 How to Access Downloads</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #229954;">Opening the Download Manager</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Click the <strong>Downloads</strong> icon in the toolbar</li>
                    <li>Or go to <strong>View → Downloads</strong> in the menu</li>
                    <li>Download window opens showing all current and past downloads</li>
                </ul>

                <h3 style="color: #229954;">Download Location</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Files are saved to: <code>Downloads/Jasmine/</code></li>
                    <li>Organized in your system's default Downloads folder</li>
                    <li>Automatic folder creation if it doesn't exist</li>
                </ul>
            </div>

            <h2 style="color: #f39c12; border-bottom: 2px solid #f39c12; padding-bottom: 5px;">📊 Download Progress Tracking</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #d68910;">Real-time Information</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>File name and size:</strong> Clear identification of what's downloading</li>
                    <li><strong>Progress bar:</strong> Visual representation of download completion</li>
                    <li><strong>Speed indicator:</strong> Current download speed (KB/s, MB/s)</li>
                    <li><strong>Time remaining:</strong> Estimated completion time</li>
                    <li><strong>Status updates:</strong> Starting, downloading, completed, cancelled</li>
                </ul>

                <h3 style="color: #d68910;">Progress Display</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Percentage completion with visual progress bar</li>
                    <li>Downloaded size vs. total file size</li>
                    <li>Dynamic speed calculations updated every second</li>
                    <li>Color-coded status indicators</li>
                </ul>
            </div>

            <h2 style="color: #9b59b6; border-bottom: 2px solid #9b59b6; padding-bottom: 5px;">🎛️ Download Controls</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #8e44ad;">During Download</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Cancel Button:</strong> Stop active downloads immediately</li>
                    <li><strong>Open Folder:</strong> Access download directory anytime</li>
                    <li><strong>Progress Monitoring:</strong> Watch real-time progress</li>
                </ul>

                <h3 style="color: #8e44ad;">After Download</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Open File:</strong> Launch downloaded file directly</li>
                    <li><strong>Open Folder:</strong> Navigate to file location</li>
                    <li><strong>Remove from List:</strong> Clean up download history</li>
                </ul>
            </div>

            <h2 style="color: #1abc9c; border-bottom: 2px solid #1abc9c; padding-bottom: 5px;">🗂️ File Organization</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #16a085;">Automatic Organization</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>All downloads saved to dedicated Jasmine folder</li>
                    <li>Automatic duplicate filename handling</li>
                    <li>Files renamed with numbers: <code>file.pdf</code>, <code>file (1).pdf</code>, <code>file (2).pdf</code></li>
                    <li>Preserves original file extensions</li>
                </ul>

                <h3 style="color: #16a085;">Smart Naming</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Uses original filename from website</li>
                    <li>Fallback to "download" if no name available</li>
                    <li>Prevents file overwrites automatically</li>
                </ul>
            </div>

            <h2 style="color: #e67e22; border-bottom: 2px solid #e67e22; padding-bottom: 5px;">🧹 Download Management</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #d35400;">Window Controls</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Clear Finished:</strong> Remove completed/cancelled downloads from list</li>
                    <li><strong>Open Downloads Folder:</strong> Quick access to download directory</li>
                    <li><strong>Individual Remove:</strong> Remove specific items from history</li>
                </ul>

                <h3 style="color: #d35400;">List Management</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Chronological list of all downloads</li>
                    <li>Persistent across application restarts</li>
                    <li>Easy cleanup of old downloads</li>
                    <li>Empty state message when no downloads</li>
                </ul>
            </div>

            <h2 style="color: #8e44ad; border-bottom: 2px solid #8e44ad; padding-bottom: 5px;">💡 Download States</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #7b1fa2;">Active Downloads</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Starting:</strong> Download initializing</li>
                    <li><strong>In Progress:</strong> Actively downloading with progress</li>
                    <li><strong>Speed Display:</strong> Real-time transfer rate</li>
                    <li><strong>Cancel Option:</strong> Stop button available</li>
                </ul>

                <h3 style="color: #7b1fa2;">Completed Downloads</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Completed:</strong> Successfully downloaded</li>
                    <li><strong>Cancelled:</strong> User stopped download</li>
                    <li><strong>Interrupted:</strong> Network or system error</li>
                    <li><strong>File Access:</strong> Open file/folder buttons available</li>
                </ul>
            </div>

            <h2 style="color: #d32f2f; border-bottom: 2px solid #d32f2f; padding-bottom: 5px;">⚡ Performance Features</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Efficient Memory Usage:</strong> Minimal resource consumption</li>
                    <li><strong>Background Downloads:</strong> Continue while browsing</li>
                    <li><strong>Multiple Downloads:</strong> Handle several files simultaneously</li>
                    <li><strong>Speed Calculation:</strong> Accurate transfer rate monitoring</li>
                    <li><strong>Progress Updates:</strong> Smooth, real-time progress tracking</li>
                </ul>
            </div>

            <h2 style="color: #388e3c; border-bottom: 2px solid #388e3c; padding-bottom: 5px;">🔧 Technical Details</h2>

            <div style="margin: 20px 0;">
                <h3 style="color: #2e7d32;">File Size Formatting</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Automatic unit conversion: B → KB → MB → GB</li>
                    <li>Decimal precision for readability</li>
                    <li>Speed shown as size per second</li>
                </ul>

                <h3 style="color: #2e7d32;">Time Calculations</h3>
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li>Remaining time based on current speed</li>
                    <li>Format: seconds, minutes, hours as appropriate</li>
                    <li>Dynamic updates as speed changes</li>
                </ul>
            </div>

            <h2 style="color: #5d4037; border-bottom: 2px solid #5d4037; padding-bottom: 5px;">🎯 Usage Tips</h2>

            <div style="margin: 20px 0;">
                <ul style="margin: 10px 0; padding-left: 20px;">
                    <li><strong>Monitor Progress:</strong> Keep download window open to watch progress</li>
                    <li><strong>Multiple Downloads:</strong> Start several downloads simultaneously</li>
                    <li><strong>Quick Access:</strong> Use "Open Folder" for easy file management</li>
                    <li><strong>Clean History:</strong> Regularly clear finished downloads</li>
                    <li><strong>Cancel if Needed:</strong> Stop unwanted downloads immediately</li>
                    <li><strong>File Organization:</strong> Downloads are automatically organized</li>
                </ul>
            </div>

            <div style="background-color: #fff3cd; padding: 15px; border-radius: 8px; margin: 15px 0;">
                <h3 style="color: #856404; margin-top: 0;">📁 Platform-Specific Notes</h3>
                <p><strong>Flathub Version:</strong> Shows download location in dialog</p>
                <p><strong>Standard Version:</strong> Opens file manager directly</p>
                <p style="margin-bottom: 0;"><strong>All Platforms:</strong> Downloads saved to system Downloads folder under "Jasmine" subdirectory</p>
            </div>

            <div style="background-color: #d1ecf1; padding: 15px; border-radius: 8px; margin: 25px 0;">
                <h3 style="color: #0c5460; margin-top: 0;">🌟 Integrated Experience</h3>
                <p style="margin-bottom: 0;">The download manager seamlessly integrates with your browsing experience, automatically handling all file downloads while providing full control and visibility over the download process.</p>
            </div>
        </div>
    )";
}

QString HelpMenuDialog::getSitesSessionsContent()
{
    return QString(
        "<h3>Managing Sites</h3>"
        "<p><strong>Creating a New Site:</strong></p>"
        "<ul>"
        "<li>Press the <strong>Clear</strong> button to clear existing fields</li>"
        "<li>Fill in the <strong>URL</strong> and <strong>Title</strong> (required fields)</li>"
        "<li>Optionally add <strong>Username</strong>, <strong>Password</strong>, and <strong>Comments</strong></li>"
        "<li>Press the <strong>Add</strong> button to save the site</li>"
        "</ul>"

        "<h3>Managing Sessions</h3>"
        "<p><strong>Creating a New Session:</strong></p>"
        "<ul>"
        "<li>Ensure at least one tab is open in the webview</li>"
        "<li>Select <strong>Save Current Session</strong> from the menu or toolbar</li>"
        "<li>Give your session a name and click <strong>OK</strong></li>"
        "<li>Sessions are automatically assigned a randomly generated SVG icon</li>"
        "</ul>"

        "<h3>Editing Sites and Sessions</h3>"
        "<p><strong>To edit any site or session:</strong></p>"
        "<ul>"
        "<li>Select the item you want to modify</li>"
        "<li>Enter the new values in the appropriate fields</li>"
        "<li>Press the <strong>Update</strong> button to save changes</li>"
        "<li>For sessions: Click the small <strong>Edit</strong> button next to the icon to change it, then click <strong>Update</strong></li>"
        "</ul>"
        );
}

QString HelpMenuDialog::getOnSecurityContent()
{
    return QString(
        "<h3>Security Features Overview</h3>"
        "<p>Jasmine provides several optional security features designed for your convenience. "
        "You are completely free to use or not use any of these features based on your preferences.</p>"

        "<h4>Username & Password Storage</h4>"
        "<ul>"
        "<li>Storing credentials in website entries is <strong>completely optional</strong></li>"
        "<li>Leave these fields blank if you prefer to use your own credentials manager</li>"
        "<li>Stored credentials are saved locally on your device only in binary format</li>"
        "<li>No data is transmitted over the network</li>"
        "</ul>"

        "<h4>Master Password Protection</h4>"
        "<ul>"
        "<li>Optional feature to protect access to Jasmine</li>"
        "<li>When enabled, you'll need to enter your master password on startup</li>"
        "<li>Choose a strong, memorable password and store it safely</li>"
        "<li>If forgotten, you'll need to perform a factory reset</li>"
        "</ul>"

        "<h4>Two-Factor Authentication (2FA) Manager</h4>"
        "<ul>"
        "<li>Optional convenience tool for generating TOTP codes</li>"
        "<li>Helps manage 2FA codes for your various accounts</li>"
        "<li>All secrets are stored locally on your device in binary format</li>"
        "<li>Use only if you're comfortable with local storage</li>"
        "</ul>"

        "<h4>Security Disclaimer</h4>"
        "<p><em>While every reasonable effort has been made to implement a secure framework "
        "and all sensitive info is stored in binary format within Jasmine, "
        "these features are provided as conveniences rather than guarantees. Users are responsible "
        "for deciding what information to store based on their individual security requirements and risk tolerance.</em></p>"

        "<p><strong>Recommendation:</strong> For maximum security, consider using dedicated password managers "
        "and letting your browser handle credential storage.</p>"
        );
}

QString HelpMenuDialog::getOnNewStorageSystemContent() {
    return QString(
        "<h3>New Storage System (Jasmine 1.1.0 and onwards)</h3>"
        "<p>This version of Jasmine uses a new storage system that improves performance and efficiency by using symlinks for profile data. This means that instead of copying entire profile directories, only links are created, saving disk space and speeding up operations.</p>"
        "<h4>Recommendations:</h4>"
        "<p>To fully leverage the new storage system and ensure optimal performance, we highly recommend performing a factory reset. This will clear all old data and provide a clean start. You can find the factory reset option in the 'Sessions' menu.</p>"
        "<h4>Clean up orphaned profile directories on startup:</h4>"
        "<p>As part of the new storage system, a checkbox is available in the 'Tools' menu to automatically clean up unused, orphaned profile directories at application startup. It is STRONGLY RECOMMENDED to keep this option enabled UNLESS you also have old-format sessions saved.</p>"
        "<h4>Handling Old-Format Sessions:</h4>"
        "<p>If you have existing sessions that use the old directory-based storage, you can continue to use them. However, we strongly advise against adding new tabs or saving changes to these older sessions. To ensure optimal performance and avoid potential issues, it's best to create new sessions using the new storage system.</p>"
        );
}

QString HelpMenuDialog::getOnNamedProfilesContent()
{
    return QString(R"(
        <div style="text-align: center; font-family: Arial, sans-serif;">
            <h1 style="color: #2c3e50; margin-bottom: 10px;">Named Shared Profiles</h1>
            <h3 style="color: #7f8c8d; margin-bottom: 20px;">Contextual Browsing Environments</h3>

            <div style="text-align: left; margin: 20px 0; line-height: 1.6;">
                <p>
                    Jasmine's Named Shared Profiles feature takes your browsing organization to the next level by allowing
                    multiple tabs to share the same browsing context under meaningful labels like "Work," "Home," "Shopping,"
                    or "Research." This powerful addition bridges the gap between completely isolated private profiles and a
                    single shared environment.
                </p>

                <div style="background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin: 20px 0;">
                    <h4 style="color: #2c3e50; margin-top: 0;">Examples:</h4>
                    <ul style="margin-left: 20px;">
                        <li><strong>Work Profile:</strong> Company email, project management tools, and internal documentation all share cookies and login states</li>
                        <li><strong>Personal Profile:</strong> Social media and entertainment sites kept separate from work accounts</li>
                        <li><strong>Finance Profile:</strong> Banking, investment, and budgeting tools with shared authentication</li>
                        <li><strong>Travel Profile:</strong> Airline, hotel, and rental car sites that need to exchange booking information</li>
                    </ul>
                </div>

                <p>
                    This contextual grouping maintains the perfect balance between isolation and integration: your work accounts
                    never mix with personal browsing, but related tools within each context can seamlessly communicate when needed.
                    Named Shared Profiles eliminate the all-or-nothing approach to profile isolation, giving you granular control
                    over exactly which tabs should share data with each other while maintaining barriers between different areas of your digital life.
                </p>

                <h3 style="color: #2c3e50; margin-top: 30px;">How to Use Named Profiles</h3>

                <ol style="margin-left: 20px;">
                    <li><strong>Create Profiles:</strong> In the profile area of the toolbar, in the profileSelector combobox select "New Profile..." and give it a meaningful name like "Work" or "Personal"</li>
                    <li><strong>Select a Profile:</strong> Choose your desired profile from the dropdown before launching websites</li>
                    <li><strong>Launch Websites:</strong> Any sites launched while a named profile is selected will share the same browsing context</li>
                    <li><strong>Create Sessions:</strong> Save groups of tabs launched under the same named profile for quick restoration</li>
                    <li><strong>Manage Profiles:</strong> Select "Manage Profiles..." to create, delete, or clean profile data</li>
                </ol>

                <div style="background-color: #e8f4f8; padding: 15px; border-radius: 8px; margin: 20px 0; border-left: 4px solid #3498db;">
                <p><strong>Note:</strong> Keeping the profile selector set to "Default" will use the universal shared profile,
                which is the standard shared browsing environment. Private profiles (toggled per tab via the Private Toggle Button) always remain
                completely isolated regardless of named profile selection.</p>

                <p><strong>Simply put:</strong></p>
                <ul style="margin-left: 20px;">
                    <li>To create tabs with private profile, toggle the Private Profile button ON in the toolbar.
                    This will inactivate the Named Profile selector.</li>
                    <li>To use a named shared profile, untoggle the Private Profile button (if toggled) and select a previously
                    created named shared profile from the combobox.</li>
                    <li>Or leave it set to "Default" to use the universal profile.</li>
                </ul>
                </div>



                <h3 style="color: #2c3e50; margin-top: 30px;">Profile Management</h3>
                <p>
                    You can manage your named profiles by selecting "Manage Profiles..." from the profile selector dropdown. This allows you to:
                </p>
                <ul style="margin-left: 20px;">
                    <li><strong>Create new profiles</strong> for different contexts or projects</li>
                    <li><strong>Delete profiles</strong> you no longer need (only if they're not in use by active sessions)</li>
                    <li><strong>Clean profile data</strong> to remove cookies, cache, and browsing history while keeping the profile</li>
                </ul>

                <p>
                    Each named profile maintains its own separate storage for:
                </p>
                <ul style="margin-left: 20px;">
                    <li>Cookies and login sessions</li>
                    <li>Browsing history and cache</li>
                    <li>Local storage and website data</li>
                    <li>Form data and preferences</li>
                </ul>
            </div>

            <hr style="margin: 30px 0; border: 1px solid #bdc3c7;">

            <div style="margin: 20px 0;">
                <p style="font-size: 14px; color: #7f8c8d;">
                    Organize your digital life with contextual browsing environments
                </p>
            </div>
        </div>
    )");

}

QString HelpMenuDialog::getChangelogContent()
{
    return QString(
        "<div style='font-family: Arial, sans-serif; line-height: 1.6; color: #2c3e50;'>"
        "<h2 style='color: #8e44ad; text-align: center; margin-bottom: 25px;'>Ermis Changelog</h2>"

        "<h4 style='color: #27ae60; border-bottom: 2px solid #27ae60; padding-bottom: 5px; margin-top: 25px;'>[v1.1.1] - 2026-04-20</h4>"
        "<ul style='margin-bottom: 20px;'>"
        "<li><b>PDF Steganography:</b> Hide data in PDF files using object stream embedding with compression</li>"
        "<li><b>Distributed Steganography:</b> Wikipedia-based pointer maps for decentralized message hiding</li>"
        "<li><b>Filename Preservation:</b> FNAM flag preserves original filenames across binary file transfers</li>"
        "<li><b>Auto Encryption Detection:</b> ENCR flag automatically triggers passphrase prompt on extraction</li>"
        "<li><b>Binary File Handling:</b> Auto-save with timestamp_ermis_filename format for extracted binaries</li>"
        "<li><b>Theme-Aware Info Buttons:</b> Comprehensive tooltips with feature documentation across all dialogs</li>"
        "<li><b>Dark Theme Fix:</b> Resolved theme toggle persistence issue on application restart</li>"
        "<li><b>File Dialog Fix:</b> Linux file dialogs now correctly display all files including binaries</li>"
        "<li><b>Network Coordination Warnings:</b> Added protocol/port synchronization guidance for senders</li>"
        "</ul>"

        "<h4 style='color: #c0392b; border-bottom: 2px solid #c0392b; padding-bottom: 5px; margin-top: 25px;'>[v1.1.0] - 2026-03-27</h4>"
        "<ul style='margin-bottom: 20px;'>"
        "<li><b>Network Steganography (ERTP):</b> Hide data in ICMP, DNS, UDP, and HTTP/TLS packets</li>"
        "<li><b>Text Steganography:</b> Zero-width Unicode character encoding for invisible message hiding</li>"
        "<li><b>Multi-Protocol Support:</b> Choose ICMP (ping cover), DNS (domain masquerading), UDP (speed), or HTTPS (security)</li>"
        "<li><b>Reliable Packet Delivery:</b> Sliding window protocol with ACK/NACK, CRC32 checksums, automatic retransmission</li>"
        "<li><b>Content Encryption:</b> AES-256-CBC with PBKDF2 key derivation (100,000 iterations) for all steganography modes</li>"
        "<li><b>IP Filtering:</b> Whitelist/blacklist filtering with CIDR notation support for source validation</li>"
        "<li><b>Session Management:</b> 32-bit unique IDs, configurable timeouts (1000-2000ms), 3-5 retry attempts</li>"
        "<li><b>Text Steganography UI:</b> Dedicated tab with cover text input, capacity calculator, zero-width encoding</li>"
        "<li><b>Multi-Layer Security:</b> Combine text steg + content encryption + network steg + TLS for triple protection</li>"
        "<li><b>Auto Format Detection:</b> Intelligent text/binary detection during extraction with user warnings</li>"
        "<li><b>Network Transfer Tracking:</b> Real-time progress indicators for sending and receiving data</li>"
        "<li><b>Maximum Payload:</b> 1400 bytes per network packet with session-based reassembly</li>"
        "</ul>"

        "<h4 style='color: #3498db; border-bottom: 2px solid #3498db; padding-bottom: 5px; margin-top: 25px;'>[v1.0.0] - 2026-03-15</h4>"
        "<ul style='margin-bottom: 20px;'>"
        "<li><b>Initial Release:</b> Ermis Steganography Tool</li>"
        "<li><b>Image Steganography:</b> Hide data in PNG, JPG, BMP using LSB techniques with live preview</li>"
        "<li><b>Audio Steganography:</b> Support for WAV, MP3, FLAC, OGG with FFmpeg conversion</li>"
        "<li><b>Dual Input Modes:</b> Text input and file hiding with filename preservation</li>"
        "<li><b>Content Encryption:</b> Optional AES passphrase protection with automatic detection</li>"
        "<li><b>Drag & Drop:</b> Intuitive file loading by dragging directly into the application</li>"
        "<li><b>Clipboard Integration:</b> One-click copying of extracted text</li>"
        "<li><b>Capacity Indicator:</b> Real-time display of available hiding space</li>"
        "<li><b>Audio Player:</b> Built-in playback controls for loaded audio files</li>"
        "<li><b>Smart Extraction:</b> Auto-detects encryption, file type, and text content</li>"
        "<li><b>Persistent Paths:</b> Smart directory fallbacks (Pictures -> Images, Music -> AppDir)</li>"
        "<li><b>Modular Engine Design:</b> Separate steganography engines for images and audio</li>"
        "<li><b>Cross-Platform:</b> Available for Linux, Windows, and macOS</li>"
        "</ul>"

        "<div style='background-color: #ecf0f1; padding: 15px; border-radius: 8px; margin: 20px 0;'>"
        "<h4 style='color: #2c3e50; margin-top: 0; text-align: center;'>Version Highlights</h4>"
        "<p style='margin-bottom: 10px;'><b>v1.1.1 New:</b> PDF steganography with object stream embedding, distributed steganography via Wikipedia pointer maps, filename preservation with FNAM flag, auto encryption detection with ENCR flag, theme-aware info buttons across all dialogs, dark theme persistence fix.</p>"
        "<p style='margin-bottom: 10px;'><b>v1.1.0 New:</b> Network steganography across 4 protocols with reliable delivery, text steganography using zero-width characters, content encryption with PBKDF2, multi-layer security composition, IP filtering with CIDR support.</p>"
        "<p style='margin: 0;'><b>v1.0.0 Foundation:</b> Media steganography (image and audio), content encryption, intuitive UI with drag-and-drop, automatic file type detection, modular engine architecture.</p>"
        "</div>"

        "<div style='background-color: #e8f8f5; padding: 15px; border-radius: 8px; margin: 20px 0;'>"
        "<p style='margin: 0; color: #117a65;'>"
        "<b>Built with:</b> Qt 6 Framework + FFmpeg + OpenSSL • "
        "<b>License:</b> GPL-3.0 • "
        "<b>Copyright:</b> 2026 Alamahant • "
        "<b>Repository:</b> <a href='https://github.com/alamahant/Ermis'>github.com/alamahant/Ermis</a>"
        "</p>"
        "</div>"

        "<div style='background-color: #f8f9fa; padding: 15px; border-radius: 8px; margin-top: 30px;'>"
        "<h4 style='color: #2c3e50; margin-top: 0;'>Planned for Future Releases</h4>"
        "<ul style='margin-bottom: 0;'>"
        "<li><b>Video Steganography:</b> Hide data in video files with codec support</li>"
        "<li><b>Batch Processing:</b> Hide/extract multiple files in single operation</li>"
        "<li><b>Steganalysis Tools:</b> Detect and analyze hidden data in suspicious files</li>"
        "<li><b>Additional Ciphers:</b> ChaCha20, Twofish, and other encryption algorithms</li>"
        "<li><b>Digital Signatures:</b> Authentication and integrity verification for hidden content</li>"
        "<li><b>Plugin Architecture:</b> Extensible system for custom steganography algorithms</li>"
        "<li><b>Cloud Integration:</b> Direct upload/download of stego files to cloud storage</li>"
        "</ul>"
        "</div>"

        "<div style='background-color: #fff3cd; padding: 15px; border-radius: 8px; margin-top: 30px;'>"
        "<h4 style='color: #856404; margin-top: 0;'>Technical Notes</h4>"
        "<ul style='margin-bottom: 0;'>"
        "<li><b>Content Encryption:</b> All steganography modes support optional AES-256-CBC encryption of the hidden content before embedding</li>"
        "<li><b>Network Packets:</b> ERTP packets are legitimate network traffic (ICMP echo, DNS queries, UDP datagrams, HTTPS). Only the embedded content can be optionally encrypted</li>"
        "<li><b>Zero-Width Characters:</b> Text steganography uses U+200B (Zero Width Space) for 0 and U+200C (Zero Width Non-Joiner) for 1 in binary encoding</li>"
        "<li><b>PDF Steganography:</b> Uses /ObjStm object streams with FlateDecode compression, appended as incremental updates for spec compliance</li>"
        "<li><b>Distributed Steganography:</b> Wikipedia article word positions serve as pointers, language-agnostic with JSON map format</li>"
        "<li><b>Session Management:</b> Network transfers use sliding window protocol with automatic retransmission and CRC32 validation</li>"
        "<li><b>PBKDF2 Derivation:</b> Key stretching uses 100,000 iterations for content encryption, providing OWASP-compliant key hardening</li>"
        "</ul>"
        "</div>"
        "</div>"
    );
}

QString HelpMenuDialog::getSupportusContent()
{
    return QString();

}




