# GitHub Repository Setup Guide

This guide will help you set up a GitHub remote repository for the USLI Summer Subscale Payload project.

## Prerequisites

- Git installed on your system
- GitHub account
- SSH key set up (recommended) or GitHub CLI

## Step 1: Create GitHub Repository

### Option A: Using GitHub Web Interface

1. **Go to GitHub.com** and sign in to your account
2. **Click "New repository"** or the "+" icon in the top right
3. **Fill in repository details:**
   - **Repository name**: `USLI-Summer-Subscale-Payload`
   - **Description**: `Rocket telemetry system for USLI Summer 2025 subscale payload`
   - **Visibility**: Choose Public or Private
   - **Initialize with**: 
     - ✅ Add a README file
     - ✅ Add .gitignore (choose Arduino)
     - ✅ Choose a license (MIT recommended)
4. **Click "Create repository"**

### Option B: Using GitHub CLI

```bash
# Install GitHub CLI if not already installed
# macOS: brew install gh
# Linux: sudo apt install gh

# Login to GitHub
gh auth login

you need to generate an SSH key in your github personal setting and use it
```

## Step 2: Configure Local Repository

### Check Current Git Status

```bash
# Check if git is already initialized
git status

# If not initialized, initialize git
git init
```

### Add Remote Repository

```bash
# Add the remote repository (replace YOUR_USERNAME with your GitHub username)
git remote add origin https://github.com/YOUR_USERNAME/USLI-Summer-Subscale-Payload.git

# Or using SSH (recommended)
git remote add origin git@github.com:YOUR_USERNAME/USLI-Summer-Subscale-Payload.git
```

### Verify Remote

```bash
# Check remote configuration
git remote -v

# Should show something like:
# origin  git@github.com:YOUR_USERNAME/USLI-Summer-Subscale-Payload.git (fetch)
# origin  git@github.com:YOUR_USERNAME/USLI-Summer-Subscale-Payload.git (push)
```

## Step 3: Prepare Files for Upload

### Create .gitignore File

```bash
# Create .gitignore file
cat > .gitignore << 'EOF'
# Arduino build files
build/
.pio/
*.hex
*.elf

# IDE files
.vscode/settings.json
.idea/
*.swp
*.swo

# OS files
.DS_Store
Thumbs.db

# Temporary files
temp_test/
*.tmp
*.bak

# Logs
*.log

# Dependencies
node_modules/
EOF
```

### Stage and Commit Files

```bash
# Add all files
git add .

# Check what will be committed
git status

# Make initial commit
git commit -m "feat: Add complete Arduino development environment and telemetry system

- Add arduino-cli build system with custom control scripts
- Implement rocket telemetry system with flight state detection
- Add sensor drivers (GPS, temperature, SD card) with LED status indicators
- Create VS Code integration and comprehensive documentation
- Test and verify Arduino Nano v4 functionality (79% program storage used)"
```

## Step 4: Push to GitHub

### First Push

```bash
# Push to main branch
git push -u origin main

# If your default branch is master, use:
# git push -u origin master
```

### Verify Upload

1. **Go to your GitHub repository page**
2. **Check that all files are uploaded:**
   - `app/` directory with Arduino sketches
   - `include/` directory with sensor headers
   - `src/` directory with sensor implementations
   - `README.md` with project documentation
   - `arduino.sh` and other build scripts
   - `.vscode/` directory with VS Code configuration

## Step 5: Set Up Repository Settings

### Enable GitHub Pages (Optional)

1. **Go to repository Settings**
2. **Click "Pages" in the left sidebar**
3. **Source**: Choose "Deploy from a branch"
4. **Branch**: Select `main` and `/docs` folder
5. **Click "Save"**

### Add Repository Topics

1. **Go to repository main page**
2. **Click the gear icon next to "About"**
3. **Add topics:**
   - `arduino`
   - `rocket`
   - `telemetry`
   - `usli`
   - `c++`
   - `embedded-systems`

### Set Repository Description

Update the repository description to:
```
🚀 Rocket telemetry system for USLI Summer 2025 subscale payload. Features Arduino Nano v4, multiple sensors, SD card logging, and professional development environment.
```

## Step 6: Collaboration Setup

### Add Collaborators (if needed)

1. **Go to repository Settings**
2. **Click "Collaborators and teams"**
3. **Click "Add people"**
4. **Enter GitHub usernames or email addresses**

### Set Up Branch Protection (Recommended)

1. **Go to repository Settings**
2. **Click "Branches"**
3. **Add rule for `main` branch:**
   - ✅ Require pull request reviews
   - ✅ Require status checks to pass
   - ✅ Include administrators

## Step 7: Continuous Integration (Optional)

### Create GitHub Actions Workflow

Create `.github/workflows/arduino-build.yml`:

```yaml
name: Arduino Build

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup Arduino CLI
      uses: arduino/setup-arduino-cli@v1
      with:
        version: latest
    
    - name: Install Libraries
      run: |
        arduino-cli lib install "TinyGPSPlus"
        arduino-cli lib install "OneWire"
        arduino-cli lib install "DallasTemperature"
        arduino-cli lib install "SD"
    
    - name: Build Project
      run: |
        arduino-cli compile --fqbn arduino:avr:nano app
```

## Troubleshooting

### SSH Key Issues

```bash
# Generate SSH key if needed
ssh-keygen -t ed25519 -C "your_email@example.com"

# Add to SSH agent
ssh-add ~/.ssh/id_ed25519

# Copy public key to GitHub
cat ~/.ssh/id_ed25519.pub
# Add this to GitHub Settings > SSH and GPG keys
```

### Permission Issues

```bash
# Check SSH connection
ssh -T git@github.com

# Should show: Hi username! You've successfully authenticated...
```

### Large File Issues

```bash
# If you have large files, use Git LFS
git lfs install
git lfs track "*.hex"
git lfs track "*.elf"
git add .gitattributes
```

## Next Steps

1. **Update README.md** with your specific repository links
2. **Add project badges** to README (build status, version, etc.)
3. **Create Issues** for future development tasks
4. **Set up project milestones** for development phases
5. **Configure branch naming conventions** for team development

## Repository Structure After Setup

```
USLI-Summer-Subscale-Payload/
├── .github/                    # GitHub Actions workflows
├── .vscode/                    # VS Code configuration
├── app/                        # Arduino sketches
├── docs/                       # Documentation
├── include/                    # Sensor headers
├── src/                        # Sensor implementations
├── tests/                      # Test sketches
├── .gitignore                  # Git ignore rules
├── arduino.sh                  # Arduino control script
├── detect_port.sh              # Port detection script
├── test_arduino.sh             # Test suite
├── platformio.ini              # PlatformIO config
├── Makefile                    # Make build system
├── README.md                   # Project documentation
├── INSTALL.md                  # Installation guide
└── GITHUB_SETUP.md            # This file
```

Your repository is now ready for collaboration and development! 🚀
