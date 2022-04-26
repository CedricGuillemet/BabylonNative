/// <reference path="../../node_modules/babylonjs/babylon.module.d.ts" />
/// <reference path="../../node_modules/babylonjs-loaders/babylonjs.loaders.module.d.ts" />
/// <reference path="../../node_modules/babylonjs-materials/babylonjs.materials.module.d.ts" />
/// <reference path="../../node_modules/babylonjs-gui/babylon.gui.module.d.ts" />


var engine = new BABYLON.NativeEngine();
var scene = new BABYLON.Scene(engine);

BABYLON.Tools.Log("Loaded");
/*
scene.createDefaultCamera(true, true, true);
scene.activeCamera.alpha += Math.PI;

if (ibl) {
    scene.createDefaultEnvironment({ createGround: false, createSkybox: false });
}
else {
    scene.createDefaultLight(true);
}
*/

var camera = new BABYLON.ArcRotateCamera("camera", 3.73, 1.28, 0.75, new BABYLON.Vector3(0.,0.5,0.), scene);
camera.lowerRadiusLimit = 2.0;
camera.upperRadiusLimit = 2.0;

// This attaches the camera to the canvas
//camera.attachControl(canvas, true);
/*
// This creates a light, aiming 0,1,0 - to the sky (non-mesh)
var light = new BABYLON.HemisphericLight("light", new BABYLON.Vector3(0, 1, 0), scene);

// Default intensity is 1. Let's dim the light a small amount
light.intensity = 0.7;
*/

// lighting
const dirLight = new BABYLON.DirectionalLight("dirLight", new BABYLON.Vector3(0.47, 0.0, -0.86), scene);
dirLight.diffuse = BABYLON.Color3.FromInts(255, 251, 199);
dirLight.intensity = 3;

var envCube = BABYLON.CubeTexture.CreateFromPrefilteredData("App:///Assets/environment.env", scene);
envCube.name = "environment";
envCube.gammaSpace = false;
envCube.rotationY = 1.977;

scene.environmentTexture = envCube;

// glow
const glowLayer = new BABYLON.GlowLayer("glowLayer", scene);
glowLayer.intensity = 0.5;

let skybox = BABYLON.SceneLoader.ImportMeshAsync("", "app:///Assets/", "starsGeo.glb").then(() => {
    const starfield = scene.getMeshByName("starsGeo");
    starfield.scaling = new BABYLON.Vector3(4500, 4500, 4500);

    BABYLON.NodeMaterial.ParseFromFileAsync("", "app:///Assets/starfieldShader.json", scene).then((nodeMaterial) => {
        starfield.material = nodeMaterial;
        var starfieldTextureBlock = nodeMaterial.getBlockByName("emissiveTex");
        starfieldTextureBlock.texture = new BABYLON.Texture("app:///Assets/starfield_panorama_texture.jpg", scene, false, false);;

        let spaceScene = BABYLON.SceneLoader.ImportMeshAsync("", "app:///Assets/", "nativeStaticScene.glb").then(() => {

            let asteroids = ["asteroidModule2_i30", "asteroidGroup187", "asteroidModule4_i82", "asteroidGroup106", "asteroidGroup84", "asteroidGroup74", "asteroidGroup73", "asteroidGroup65", "asteroidGroup87", "asteroidGroup61", "asteroidGroup104", "asteroidGroup105", "asteroidGroup127", "asteroidGroup141", "asteroidGroup135", "asteroidGroup129", "asteroidGroup126", "asteroidGroup100", "asteroidGroup63", "asteroidGroup81", "asteroidGroup72", "asteroidGroup102", "asteroidGroup26", "asteroidGroup235", "asteroidGroup78", "asteroidGroup88", "asteroidGroup3", "asteroidGroup7", "asteroidModule3_i148", "asteroidModule3_i117", "asteroidModule2_i107"];

            const frameRate = 10;
            let rotationSpeedMin = 200;
            let rotationSpeedMax = 700

            for (each in asteroids) {
                let asteroid = scene.getMeshByName(asteroids[each])
                if (!asteroid) {
                    asteroid = scene.getTransformNodeByName(asteroids[each])
                }

                asteroid.rotationQuaternion = null;

                let parentTransform = new BABYLON.TransformNode("parent", scene);
                parentTransform.parent = asteroid.parent;
                parentTransform.position.copyFrom(asteroid.position);
                asteroid.position.setAll(0);
                asteroid.parent = parentTransform;
                parentTransform.rotation = new BABYLON.Vector3(
                    BABYLON.Scalar.RandomRange(-Math.PI, Math.PI),
                    BABYLON.Scalar.RandomRange(-Math.PI, Math.PI),
                    BABYLON.Scalar.RandomRange(-Math.PI, Math.PI)
                );

                let rotationSpeed = Math.floor(Math.random() * (rotationSpeedMax - rotationSpeedMin + 1)) + rotationSpeedMin;

                const randomRotation = new BABYLON.Animation("rotation", "rotation.x", frameRate, BABYLON.Animation.ANIMATIONTYPE_FLOAT, BABYLON.Animation.ANIMATIONLOOPMODE_CYCLE);
                const keys = [];
                keys.push({
                    frame: 0,
                    value: BABYLON.Tools.ToRadians(0),
                });
                keys.push({
                    frame: rotationSpeed,
                    value: BABYLON.Tools.ToRadians(360),
                });
                randomRotation.setKeys(keys);
                asteroid.animations.push(randomRotation);
                scene.beginAnimation(asteroid, 0, rotationSpeed, true);
            }

            for (each in scene.transformNodes) {
                let randomScale = (Math.random() * 0.4) + 0.9;
                randomScale *= randomScale;
                //randomScale *= randomScale;
                scene.transformNodes[each].scaling = new BABYLON.Vector3(randomScale, randomScale, randomScale);
            }

            scene.getTransformNodeByName("asteroidGroup104").scaling = new BABYLON.Vector3(1.3, 1.3, 1.3);
            scene.getTransformNodeByName("asteroidGroup104").parent.scaling = new BABYLON.Vector3(1.22, 1.22, 1.22);
            scene.getTransformNodeByName("asteroidGroup3").scaling = new BABYLON.Vector3(1, 1, 1);
            scene.getTransformNodeByName("asteroidGroup3").parent.scaling = new BABYLON.Vector3(1, 1, 1);
            scene.getTransformNodeByName("asteroidGroup105").scaling = new BABYLON.Vector3(1, 1, 1);
            scene.getTransformNodeByName("asteroidGroup105").parent.scaling = new BABYLON.Vector3(1.25, 1.25, 1.25);
            scene.getTransformNodeByName("asteroidGroup100").scaling = new BABYLON.Vector3(1, 1, 1);
            scene.getTransformNodeByName("asteroidGroup100").parent.scaling = new BABYLON.Vector3(1.25, 1.25, 1.25);
            scene.getTransformNodeByName("asteroidGroup102").scaling = new BABYLON.Vector3(1.52, 1.52, 1.52);
            scene.getTransformNodeByName("asteroidGroup102").parent.scaling = new BABYLON.Vector3(1.29, 1.29, 1.29);
            scene.getTransformNodeByName("asteroidGroup87").scaling = new BABYLON.Vector3(1.03, 1.03, 1.03);
            scene.getTransformNodeByName("asteroidGroup87").parent.scaling = new BABYLON.Vector3(1.54, 1.54, 1.54);
            scene.getTransformNodeByName("asteroidGroup84").scaling = new BABYLON.Vector3(1.28, 1.28, 1.28);
            scene.getTransformNodeByName("asteroidGroup84").parent.scaling = new BABYLON.Vector3(1.47, 1.47, 1.47);
            scene.getMeshByName("asteroidModule2_i30").parent.scaling = new BABYLON.Vector3(1.13, 1.13, 1.13);
            scene.getMeshByName("asteroidModule1_i13").parent.scaling = new BABYLON.Vector3(1, 1, 1);

            camera.checkCollisions = true;
            camera.collisionRadius = new BABYLON.Vector3(1.5, 1.5, 1.5);
            camera.minZ = 0.01;

            let arMode = false;
            let arAssets = ["__root__", "enemyShip", "heroShip", "laser1", "laser2", "laser3", "atmosphereSphere_mesh", "planet_mesh", "asteroidModule2_i30", "asteroidModule1_i91", "asteroidModule2_i106", "asteroidModule3_i115", "asteroidModule1_i108", "asteroidModule1_i109", "asteroidModule2_i124", "asteroidModule2_i125", "asteroidModule2_i126", "asteroidModule3_i141", "asteroidModule3_i142", "asteroidModule4_i119", "asteroidModule4_i120", "asteroidModule4_i121", "asteroidModule1_i106", "asteroidModule2_i122", "asteroidModule3_i137", "asteroidModule4_i117", "asteroidModule1_i93", "asteroidModule1_i94", "asteroidModule2_i109", "asteroidModule3_i118"];
            // let arAssets = [];
            //let advancedTexture = BABYLON.GUI.AdvancedDynamicTexture.CreateFullscreenUI("UI");
            let atmosphere = scene.getMeshByName("atmosphereSphere_mesh");
            let oplanet = scene.getMeshByName("planet_mesh");
            atmosphere.setEnabled(false);
            oplanet.setEnabled(false);

            // lasers
            var mat = new BABYLON.StandardMaterial("laserMat", scene);
            mat.disableLighting = true;
            mat.emissiveColor = BABYLON.Color3.Red();
            mat.alphaMode = BABYLON.Engine.ALPHA_ADD;
            mat.alpha = 0.33;
            scene.getMeshByName("laser1").material = mat;
            scene.getMeshByName("laser1").scaling = new BABYLON.Vector3(3, 1, 3);
            scene.getMeshByName("laser2").scaling = new BABYLON.Vector3(3, 1, 3);
            scene.getMeshByName("laser3").scaling = new BABYLON.Vector3(3, 1, 3);
            scene.getMeshByName("laser2").material = mat;
            scene.getMeshByName("laser3").material = mat;



            var sun = new BABYLON.VolumetricLightScatteringPostProcess('volumetric', 1.0, camera, undefined, 100, BABYLON.Texture.BILINEAR_SAMPLINGMODE, scene.getEngine(), false);
            sun.mesh.material.diffuseTexture = new BABYLON.Texture("app:///Assets/sun.png", scene, true, false, BABYLON.Texture.BILINEAR_SAMPLINGMODE);
            sun.mesh.material.diffuseTexture.hasAlpha = true;
            sun.mesh.scaling = new BABYLON.Vector3(50, 50, 50);

            glowLayer.addExcludedMesh(sun.mesh);

            
            var planet = BABYLON.MeshBuilder.CreatePlane("planetPlane", { size: 100 }, scene);
            planet.position.x = 200;
            planet.billboardMode = BABYLON.Mesh.BILLBOARDMODE_ALL | BABYLON.Mesh.BILLBOARDMODE_USE_POSITION;
            
            var planetMaterial = new BABYLON.StandardMaterial("", scene);
            planetMaterial.backFaceCulling = false;
            planetMaterial.disableLighting = true;
            
            planetMaterial.diffuseTexture = new BABYLON.Texture("app:///Assets/planetImage.png", scene, true, false, BABYLON.Texture.BILINEAR_SAMPLINGMODE);
            planetMaterial.emissiveColor = new BABYLON.Color3(1, 1, 1);
            planetMaterial.alphaMode = BABYLON.Engine.ALPHA_COMBINE;
            planetMaterial.diffuseTexture.hasAlpha = true;
            planetMaterial.useAlphaFromDiffuseTexture = true;
            glowLayer.addExcludedMesh(planet);
            /*
            planet.alphaIndex = 5;
            */
            planet.material = planetMaterial;

            engine.runRenderLoop(function () {
                camera.alpha += 0.001;

                sun.mesh.position.copyFrom(scene.activeCamera.position);
                sun.mesh.position.x -= 0.47 * 300;
                sun.mesh.position.y += 0.00 * 300;
                sun.mesh.position.z += 0.86 * 300;


                scene.render();
            });
        });
    });
});