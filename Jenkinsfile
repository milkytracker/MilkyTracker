def email_receivers = "marlon@eevul.org"
def build = env.BUILD_NUMBER.toInteger() - 5
def notify(status, email_receivers){
	emailext (
		to: "${email_receivers}",
		mimeType: 'text/html',
		subject: "${status}: Job '${env.JOB_NAME} [${env.BUILD_NUMBER}]'",
		body: """<p>${status}: Job <a href='${env.BUILD_URL}'>${env.JOB_NAME} [${env.BUILD_NUMBER}]</a></p>"""
	)
}

def buildStep(config, ext) {
	sh "make clean config=$config"
	sh "make config=$config -j8"
	sh "mv bin/milkytracker.$ext publishing/deploy/MilkyTracker/"
	//sh "cp publishing/amiga-spec/MilkyTracker.info publishing/deploy/MilkyTracker/MilkyTracker.$ext.info"
}

env.PATH = "/usr/local/bin:/usr/bin:/bin:/opt/m68k-amigaos/bin:/opt/ppc-amigaos/bin:/opt/ppc-morphos/bin:/opt/ppc-warpos/bin:/opt/x86-aros/bin/linux-x86_64/tools:/opt/x86-aros/bin/linux-x86_64/tools/crosstools"

node {
	try{
		stage('Checkout and pull') {
			properties([pipelineTriggers([githubPush()])])
			if (env.CHANGE_ID) {
				echo 'Check pull request'
			} else {
				git url: 'https://github.com/AmigaPorts/MilkyTracker.git', credentialsId: '8e82b347-73ca-460c-9d51-396c844cc636', branch: '$BRANCH_NAME'
			}
		}

		stage('Clean workspace') {
			sh './clean'
		}

		stage('Generate makefiles') {
			sh './build_gmake'
		}

		stage('Generate publishing directories') {
			sh "rm -rfv publishing/deploy/*"
			sh "mkdir -p publishing/deploy/MilkyTracker"
		}

		stage('Build AmigaOS 3.x version') {
			buildStep('release_m68k-amigaos', '68k')
		}

		stage('Build AmigaOS 4.x version') {
			buildStep('release_ppc-amigaos', 'os4')
		}

		stage('Build MorphOS 3.x version') {
			buildStep('release_ppc-morphos', 'mos')
		}

		stage('Build AROS x86 ABI-v1 version') {
			buildStep('release_i386-aros', 'aros-abiv1')
		}

		stage('Deploying to stage') {
		  sh "ls -l publishing/deploy/MilkyTracker"

		}
	} catch(err) {
		currentBuild.result = 'FAILURE'
		notify('Build failed', email_receivers)
	}
}
