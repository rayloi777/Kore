let project = new Project('Cube-Test');

await project.addProject(findKore());

project.addFile('sources/**');
project.addKongDir('shaders');
project.setDebugDir('deployment');

project.addIncludeDir('../../build/Kong-osx-metal');
project.addFile('../../build/Kong-osx-metal/kong.m');

project.flatten();

resolve(project);
